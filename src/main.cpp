#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <GLES3/gl3.h>

#include "FastNoiseLite.h"
#include "voronoi.h"
#undef assert

namespace gui = ImGui;

#include "util.h"
#include "linalg.h"
#include "image.h"
#include "rand.h"
#include "buffer.h"
#include "asset.h"
#include "renderer.h"
#include "slice.h"



static void glfw_error_callback(int error, const char *msg) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, msg);
}

Color rand_rgb() {
    return Color(randi_range(0, 255),randi_range(0, 255),randi_range(0, 255));
}

inline Color color_lerp(Color a, Color b, f32 delta) {
    auto l = lerp(color_vec4(a), color_vec4(b), delta);
    return Color::xyz(l.x, l.y, l.z);
}

template<typename T>
inline Vec2i vsize(const Array2D<T> &arr) {
    return Vec2i(arr.size.x, arr.size.y);
}

template<typename T>
T quadratic_bezier(const T &a, const T &b, const T &c, f32 t) {    
    const f32 t0 = (1.-t)*(1.-t);
    const f32 t1 = t*(1.-t);
    const f32 t2 = t*t;
    return t0*a+t1*b+t1*b+t2*c;
}
template<>
Color quadratic_bezier<Color>(const Color &a, const Color &b, const Color &c, f32 t) {
    auto vec = quadratic_bezier<Vec3>(color_vec3(a), color_vec3(b), color_vec3(c), t);
    return Color::xyz(vec.x, vec.y, vec.z);
}

f32 normalize(f32 v, f32 min_v, f32 max_v) {
    return (v - min_v)/(max_v - min_v);
}

GLFWwindow *init_glfw_and_imgui();
void test_draw(Framebuffer &fb, Shader *shader);


struct Dijkstra_Map {
    static constexpr i32 NONE = I32_MAX;
    i32 max_value = I32_MIN;
    i32 min_value = I32_MAX;
    Array2D<i32> grid;    
    struct Source {
        Vec2i cell;
        i32 value;
    };
    Array<Source> sources;
};


// Very bad approach
// start must be legal
void dont_use_grid_flood(Dijkstra_Map &dm, Vec2i start, i32 value) {
    i32 *entry = &dm.grid.get(start.x, start.y);
    if(*entry <= value) return;
    *entry = value;

    ++value;
    for(i32 i = 0; i < vec2i_omni_direction_count(); ++i) {
        Vec2i cell = start + vec2i_get_omni_direction(i);
        if(!dm.grid.is_valid(cell.x, cell.y)) continue;
        dont_use_grid_flood(dm, cell, value);
    }
    return;
}

void make_dijkstra_map(Dijkstra_Map &self, Array<i32> &sources_values) {
    self.grid.resize(50, 50, self.NONE);
    auto start = vsize(self.grid)/2;    
    self.max_value = I32_MIN;
    self.min_value = I32_MAX;    
    Array<Vec2i> sources = {start, {0,0}, {30, 5}, {12,40}};
    release_assert(sources.size() == sources_values.size());
    // Array<i32> sources_values = {-4, 0};
    // Array<Vec2i> sources = {start, {0,0}};
    // O(X*Y*SOURCES) ~ O(n^2 * SOURCES)    
    ForRange(y, 0, self.grid.size.y) {
        ForRange(x, 0, self.grid.size.x) {
            Vec2i cell = {x, y};
            i32 *entry = &self.grid(x,y);
            ForRange(i, 0, sources.size()) {
                auto &src = sources[i];
                
                i32 val = sources_values[i] + maxelem(abs(src - cell));
                if(val < *entry) {
                    *entry = val;
                }                
            }
            self.max_value = max(self.max_value, *entry);
            self.min_value = min(self.min_value, *entry);
        }
    }
    self.sources.resize(sources.size());
    ForRange(i, 0, sources.size()) {
        self.sources[i] = {.cell = sources[i], .value = sources_values[i]};
    }
}
void draw_dijkstra_map(Framebuffer &fb, Dijkstra_Map &self, Shader *shader, bool use_colors = false, bool inverse_colors = false) {
    // fb.modulate = {0,0,0,1};
    use_framebuffer(fb);    
    const Vec2 step = Vec2(1, 1)/vsize(self.grid);    
    const Vec2 off = step/2.f;
    const Vec2 scale = Vec2(1, 1)/Vec2(self.grid.size.x, self.grid.size.y);

    
    Color A = Color(255, 0, 0), B = Color(128, 0, 128), C = Color(0, 0, 255);
    
    
    f32 val_range = self.max_value - self.min_value;
    ForRange(y, 0, self.grid.size.y) {
        ForRange(x, 0, self.grid.size.x) {
            auto pos = step * Vec2(x, y);
            auto grid_val = self.grid(x,y);
            // Linearize them into [0,1]; grid_val can be negative!
            f32 val = f32(grid_val - self.min_value)/val_range;
            if(inverse_colors) {
                val = 1. - val;
            }
            Color color;            
            if(use_colors) {
                color = quadratic_bezier(A, B, C, val);
            } else {
                color = Color(255, 255, 255)*val;
            }            
            draw_texture(fb, off+pos, scale, color, shader);
        }
    }
    for(auto &it: self.sources) {
        draw_texture(fb, step*Vec2(it.cell.x, it.cell.y) + off, scale, Color(252, 186, 3), shader);
    }
    use_main_framebuffer();
}
enum struct Cell_Type {
    Normal,
    Wall,
};
struct Cellular_Automata {
    Array2D<Cell_Type> grid;
    Array2D<Cell_Type> _work;
};
// @todo Memory reuse
void make_cellular_automata(Cellular_Automata &self, u64 seed = 0, isize iterations = 5, Vec2i size = {100, 100}) {
    set_global_random_engine_seed(seed);
    using enum Cell_Type;
    if(vsize(self.grid) != size) {
        self.grid.resize(size.x, size.y);
        self._work.resize(size.x, size.y);
    }
    for(auto &c: self.grid.data) {
        auto r = randf();
        if(r > 0.5) {
            c = Normal;
        } else {
            c = Wall;
        }
    }
    
    Array2D<Cell_Type> *A = &self.grid;
    Array2D<Cell_Type> *B = &self._work;
    ForRange(iteration, 0, iterations) {
        // for(auto cell: SVec2i({1,1}, size - Vec2i{1,1})) {
        for(auto cell: iterate_vec2i({0,0}, size)) {
            // @todo There should really be a smarter way to do this.
            // Just copy the edges onces during construction?
            if(cell.x == 0 || cell.y == 0 || cell.x == size.x-1 || cell.y == size.y-1) {
                B->get(cell.x, cell.y) = A->get(cell.x, cell.y);
                continue;
            }
            i32 count = 0;
            ForRange(y, cell.y-1, cell.y+2) {
                ForRange(x, cell.x-1, cell.x+2) {
                    if(x == cell.x && y == cell.y) continue;

                    if(A->get(x,y) == Wall) {
                        ++count;
                    }
                }
            }
            Cell_Type entry;
            if(count == 0 || count >= 5) {
                entry = Wall;
            } else {
                entry = Normal;
            }
            B->get(cell.x, cell.y) = entry;
        }
        swap(A, B);        
    }
    if(B == &self.grid) {
        ForRange(i, 0, self._work.count()) self.grid[i] = self._work[i];
    }
    // @todo
    bool filter = false;
    if(!filter) return;
    ForRange(i, 0, self._work.count()) self._work[i] = self.grid[i];
    for(auto cell: iterate_vec2i({1,1}, size-Vec2i{1,1})) {
        i32 count = 0;
        if(self._work.get(cell.x+1, cell.y) == Wall) ++count;
        if(self._work.get(cell.x-1, cell.y) == Wall) ++count;
        if(self._work.get(cell.x, cell.y+1) == Wall) ++count;
        if(self._work.get(cell.x, cell.y-1) == Wall) ++count;
        if(count <= 3) {
            self.grid.get(cell.x, cell.y) = Normal;
        }
    }
}
Texture draw_cellular_automata(Framebuffer &fb, Cellular_Automata &self, Shader *shader) {
    fb.modulate = {0,0,0,1};
    use_framebuffer(fb);
    
    const auto size_vec =  Vec2(self.grid.size.x, self.grid.size.y);
    const Vec2 step = Vec2(1, 1)/size_vec;
    const Vec2 off = step/2.f;
    const Vec2 scale = 0.5f * Vec2(1, 1)/size_vec;
    ForRange(y, 0, self.grid.size.y) {
        ForRange(x, 0, self.grid.size.x) {
            // auto cell = Vec2i{x, y};
            auto pos = step * Vec2(x, y);
            
            auto it = self.grid(x,y);
            if(it == Cell_Type::Normal) {                
                draw_texture(fb, off+pos, scale, Color(0,0,0), shader);
            } else if(it == Cell_Type::Wall) {
                
                draw_texture(fb, off+pos, scale, Color(123,0,50), shader);
            }
        }
    }
    use_main_framebuffer();
    return fb.color;
}
struct Room {
    i32 uid;
    Vec2i cell;
    Vec2i size;
    Color color;    
};
struct Path {
    Vec2i start;
    Vec2i end;
    i32 start_room;
    i32 end_room;
};
struct Simple_Room_Placement {
    Array2D<i32> data;
    Map<i32, Room> rooms;
    // i32 room_count = 0;
    Array<Path> paths;
    inline Vec2i size() {
        return {data.size.x, data.size.y};
    }
};
inline Vec2i rand_vec2i(i32 x, i32 y) {
    return {randi32_range(0, x), randi32_range(0, y)};
}
inline Vec2i rand_vec2i(i32 xa, i32 xb, i32 ya, i32 yb) {
    return {randi32_range(xa, xb), randi32_range(ya, yb)};
}
// Seems like a bad idea 
//inline Vec2i rand_vec2i(Vec2i a, Vec2i b) { return {randi_range(a.x, a.y), randi_range(b.x, b.y)};}

bool try_insert(Array2D<i32> &arr, Vec2i cell, Vec2i size, i32 value) {
    Vec2i to = cell + size;
    ForRange(y, cell.y, to.y) {
        ForRange(x, cell.x, to.x) {            
            if(!arr.is_valid(x, y) || arr(x,y) >= 0) {
                return false;
            }
        }
    }
    ForRange(y, cell.y, to.y) {
        ForRange(x, cell.x, to.x) {            
            arr(x,y) = value;
        }
    }
    return true;
}

Maybe<Path> try_build_path(Simple_Room_Placement &self, Room &room, const Vec2i start, Set<Pair<i32, i32>> &connected) {
    i32 start_direction = randi_range(0, 3);    
    ForRange(idir, 0, 4) {
        auto step = vec2i_get_direction((start_direction+idir) % 4);
        for(Vec2i cell = start; self.data.is_valid(cell.x, cell.y); cell += step) {
            auto it = self.data(cell.x, cell.y);
            // Check if it is another room
            if(it >= 0 && it != room.uid) {
                // Can't build paths through another room                
                if(connected.contains({room.uid, it})) break;                
                // See @todo
                connected.insert({room.uid, it});
                connected.insert({it, room.uid});
                
                return Path{.start = start, .end = cell, .start_room = room.uid, .end_room = it};
            }
        }

    }
    return {};
}

void build_paths(Simple_Room_Placement &self, isize max_paths) {
    i32 iteration = 0;
    auto rooms = to_array(self.rooms);
    // @todo Use symmetric hash
    Set<Pair<i32, i32>> connected;
    while(self.paths.size() < max_paths && self.paths.size() < (isize)self.rooms.size() && iteration < 1000) {
        auto ridx = randi_range(0, rooms.size()-1);
        auto &room = rooms[ridx];
        auto to = room.cell + room.size - Vec2i{1,1};
        auto r_cell = rand_vec2i(room.cell.x, to.x, room.cell.y, to.y);
        auto path = try_build_path(self, room, r_cell, connected);
        if(path.has_value()) {
            self.paths.push_back(*path);
        }
        ++iteration;
    }
}
Simple_Room_Placement make_simple_room_placement(u64 seed = 0, isize max_paths = 20) {
    set_global_random_engine_seed(seed);

    Simple_Room_Placement self;
    Vec2i g_size = {100, 100};
    self.data.resize(g_size.x, g_size.y, -1);
    ForRange(i, 0, 50) {
        auto cell = rand_vec2i(g_size.x, g_size.y);
        auto room_size = rand_vec2i(2, 8, 2, 8);
        if(try_insert(self.data, cell, room_size, i)) {
            self.rooms[i] = Room{.uid = i, .cell = cell, .size = room_size, .color = rand_rgb()};
        }
    }
    build_paths(self, max_paths);
    return self;
}

Texture draw_srp(Framebuffer &fb, Simple_Room_Placement &srp, Shader *shader) {
    fb.modulate = {0,0,0,1};
    use_framebuffer(fb);        
    const Vec2 step = Vec2(1, 1)/Vec2(srp.size());
    const Vec2 scale = 0.5f * Vec2(1, 1)/Vec2(srp.size());

    for(auto &[key, room]: srp.rooms) {
        Vec2i cell = room.cell;
        Vec2i to = room.cell + room.size;
        ForRange(y, cell.y, to.y) ForRange(x, cell.x, to.x) {
            auto pos = step * Vec2(x, y);
            draw_texture(fb, pos, scale, room.color, shader);
        }
    }
    for(auto &path: srp.paths) {
        auto cell_step = path.end - path.start;
        cell_step.x = 1 * sgn(cell_step.x);
        cell_step.y = 1 * sgn(cell_step.y);
        
        auto cell = path.start;
        do {
            auto pos = step * Vec2(cell);
            draw_texture(fb, pos, scale, Color(100, 100, 100), shader);
            cell += cell_step;
        } while(cell != path.end);
    }
    use_main_framebuffer();
    return fb.color;
}
struct Random_Walk {
    Array<i32> data;
    Vec2i size;
    i32 walk_count;
};
Random_Walk random_walk(Vec2i size, i32 iter_count, f32 step_scale, u64 seed) {

    auto start = size/2;
    
    Array<i32> arr;
    
    arr.resize(size.x*size.y);
    i32 steps = f32(size.x*size.y) * step_scale;
    set_global_random_engine_seed(seed);
    ForRange(i, 0, iter_count) {        
        auto pos = start;
        ForRange(step, 0, steps) {            
            if(!contains(pos, size)) {
                break;
            }
            auto idx = index2d(pos, size.x);
            arr[idx] += 1;
            auto r = randi_range(0,3);
            auto d = vec2i_get_direction(r);
            pos += d;            
        }
    }
    return {.data = arr, .size = size, .walk_count = iter_count};
}

Array<Color> make_random_walk_image(Random_Walk &rw) {
    Array<Color> colors(rw.data.size());

    Color A = Color(140, 174, 230), B = Color(68, 179, 14), C = Color(117, 62, 6);
    constexpr Buffer<f32, 3> threshold = {1.1, 0.5, 0.3};
    // f32 m = *std::max_element(rw.data.begin(), rw.data.end());
    f32 m = f32(rw.walk_count);
    // @todo Quadratic bezier curve
    For(it, i, rw.data) {        
        if(*it > 0 ) {
            f32 scale = log(1+f32(*it)/m);  
            Color color;
            
            if(scale > threshold[0]) {                
                // color = color_lerp(B, A, scale);
                color = A;
            }
            else if(scale > threshold[1]) {                
                // color = color_lerp(C, B, scale*2.);
                color = B;
            } else {
                color = C;                
            }
            if(scale > 1.) scale = 1.;
            colors[i] = scale_rgb(color, scale);
        } else {
            colors[i] = Color::Black;
        }
    }
    return colors;
}

Texture random_walk_and_load(isize iter_count = 25, f32 step_scale = 0.1, u64 seed = 0) {
    auto rw_data = random_walk({700, 700}, iter_count, step_scale, seed);
    auto rw_img_data = make_random_walk_image(rw_data);
    auto rw_image = *load_texture((byte *)&rw_img_data[0], rw_data.size.x, rw_data.size.y);
    return rw_image;
}

void draw_image(Texture &image) {
    
    // gui::Text("pointer = %x", image.internal);
    gui::Text("size = %d x %d, %d", image.size.x, image.size.y, image.internal);
    gui::Image((void*)(intptr_t)image.internal, ImVec2(image.size.x, image.size.y));
    
}

// Meant for using static strings for the gui
String make_string_buffer(isize size, const String &msg) {
    // So we can set '\0'
    assert((isize)msg.size() < size);
    String s = msg;
    s.resize(size);
    s[msg.size()] = '\0';
    return s;
}


struct Voronoi {
    VriGraph internal;
    u64 seed = 0;
    Vec2i size() {
        return Vec2i(internal.width, internal.height);
    }
    void destroy() {
        free_vri_graph(&internal);
    }
};

void make_voronoi(Voronoi &vr, u64 seed = 0, u32 points_count = 500, u16 relaxations = 5) {
    if(seed == 0) {
        seed = get_random_seed();
    }
    set_global_random_engine_seed(seed);
    vr.seed = seed;
    // @todo Allocator
    vr = {make_voronoi_random(700, 700, points_count, relaxations, (int)seed)};
}

inline Vec3 vec3(const VriPoint &p) {
    return Vec3(p.x, p.y, 0);
}

void draw_voronoi(Framebuffer &fb, Voronoi &_self, Shader *shader, bool draw_connections = false) {    
    set_global_random_engine_seed(_self.seed);
    fb.modulate = {0,0,0,1};
    use_framebuffer(fb);
    Array<Vec3> pos = {};
    Array<u32> ind = {};
    
    
    // push_tri(pos, ind, {-1,0,0}, {0,1,0}, {1,0,0});
    // push_tri(pos, ind, {1,0,0}, {-1,0,0}, {0,-1,0});
    VriGraph &self = _self.internal;

    Vec3 step = Vec3(1,1,1)/Vec3(self.width, self.height, 1);
    Vec2 step2 = Vec2(1,1)/Vec2(self.width, self.height);
    // Cheap way to produce edges
    const f32 F = 0.90;
    ForRange(inode, 0, self.node_count) {
        auto node = &self.nodes[inode];
        Vec3 point = vec3(node->point);        
        pos.resize(0);
        ind.resize(0);    

        ForRange(itri, 0, node->tri_count) {
            auto t = node->tri[itri];
            // Vertices around point;
            Vec3 a = F*step*(vec3(t.a)-point), b = F*step*(vec3(t.b)-point), c = F*step*(vec3(t.c)-point);
            push_tri(pos, ind, a, b, c);
        }
        
        auto ro =  make_render_object(pos, ind);
        auto pos = Vec2(step.x, step.y) * Vec2(node->point.x, node->point.y);
        draw_texture(ro, fb, pos, {1,1}, rand_rgb(), shader);
        destroy(ro);
    }
    if(draw_connections) {
        pos.resize(0);
        ind.resize(0);
        ForRange(inode, 0, self.node_count) {
            auto &node = self.nodes[inode];
            Vec2 point = {node.point.x, node.point.y};        
            ForRange(adj, 0, node.adj_count) {
                auto &n = self.nodes[node.adj[adj]];
                push_line(pos, ind, step2*point, step2*Vec2{n.point.x, n.point.y}, 0.002);
            }
        }    
        auto ro =  make_render_object(pos, ind);
        draw_texture(ro, fb, {0.0, 0.0}, {1,1}, Color(0, 0, 255), shader);
        destroy(ro);
    }
    use_main_framebuffer();
}

struct Noise_Generator {
    Array<Color> colors = Array<Color>(700*700);
    Vec2i size = {700, 700};
    Buffer<f32, 3> limits = {0.084, 0.278, 0.575};
    FastNoiseLite internal;
    u64 seed = 1;
    f32 freq = 0.01;
	f32 gain = 0.5;
	int octaves = 5; 
	f32 lacunarity = 2;
    FastNoiseLite::NoiseType noise_type = FastNoiseLite::NoiseType::NoiseType_OpenSimplex2;
	FastNoiseLite::FractalType fractal_type = FastNoiseLite::FractalType::FractalType_FBm;	
    void update() {
        internal.SetFrequency(freq);
		internal.SetFractalLacunarity(lacunarity);
		internal.SetFractalGain(gain);
		internal.SetFractalOctaves(octaves);
		internal.SetNoiseType(noise_type);
		internal.SetFractalType(fractal_type);
		internal.SetSeed(seed);
    }
    Noise_Generator() {
        update();        
    }
};

Texture make_noise_image(Noise_Generator &gen, bool use_colors = false) {    
    const Color A = Color(27, 1, 120), B = Color(5, 74, 36), C = Color(113, 136, 153);
    const Color DEEP = Color(8, 1, 43);
    // f32 scale = 5.;
    auto &th = gen.limits;
    for(auto cell: iterate_vec2i({0,0}, gen.size)) {
        f32 f = (1.0 + gen.internal.GetNoise(f32(cell.x), f32(cell.y)))/2.;
        // Vec2 pos = scale * Vec2(cell)/Vec2(gen.size);
        // f32 f = (1.0 + gen.internal.GetNoise(pos.x, pos.y))/2.;
        Color color;
        if(use_colors) {
            if(f <= th[0]) {
                color = DEEP;                
            } else if(f <= th[1]) {
                color = color_lerp(DEEP, A, normalize(f, th[0], th[1]));                
            } else if(f <= th[2]) {
                color = color_lerp(A, B, normalize(f, th[1], th[2]));
                
            } else {
                color = color_lerp(B, C, normalize(f, th[2], 1.));
            }
        } else {        
            color = Color::xyz(f,f,f);
        }
        gen.colors[cell.y*gen.size.x + cell.x] = color;
    }
    return *load_texture((byte *)&gen.colors[0], gen.size.x, gen.size.y);    
}
int main(int, char **) {
    
    auto window = init_glfw_and_imgui();
    release_assert(window);
    
    // *load_texture("bm.png");
    
    load_shader("default");
    auto basic_shader = load_shader("basic");
    load_shader("framebuffer");    
    
    init_renderer();

    auto srp = make_simple_room_placement();
    auto srp_fb = make_framebuffer_target({700, 700});    
    draw_srp(srp_fb, srp, basic_shader);

    Cellular_Automata ca;
    make_cellular_automata(ca);
    auto ca_fb = make_framebuffer_target({700, 700});    
    draw_cellular_automata(ca_fb, ca, basic_shader);

    Voronoi vr;
    make_voronoi(vr, 0);
    auto vr_fb = make_framebuffer_target({700, 700});    
    draw_voronoi(vr_fb, vr, basic_shader);
    defer(vr.destroy());
    
    Dijkstra_Map dm;
    static Array<i32> dm_values = {0, 0, -10, 100};
    make_dijkstra_map(dm, dm_values);
    auto dm_fb = make_framebuffer_target({700, 700});    
    draw_dijkstra_map(dm_fb, dm, basic_shader);
    
    auto rw_image = random_walk_and_load();        

    auto noise = Noise_Generator{};
    auto noise_image = make_noise_image(noise, true);

    f64 total_time = 0.;
    auto &io = gui::GetIO();
    bool show_demo_window = false;    
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while(!glfwWindowShouldClose(window)) {
        // io.WantCaptureMouse, io.WantCaptureKeyboard
        glfwPollEvents();
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }       
        assets_check_reload();

        f32 delta = io.DeltaTime;
        total_time += delta;

        
    

        use_main_framebuffer();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        gui::NewFrame();
        
        if(show_demo_window)
            gui::ShowDemoWindow(&show_demo_window);
        // gui::SameLine();
        gui::SetNextWindowSize({1920, 1080});
        gui::SetNextWindowPos({0,0});
        // ImGuiWindowFlags_NoBackground
        gui::Begin("Stuff", nullptr, ImGuiWindowFlags_NoTitleBar);
        gui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        gui::Text("Note: A seed of 0 will randomly generate one");
        constexpr const char *seed_hint = "A seed of 0 will randomly generate one";
        if (gui::BeginTable("split", 2)) {            
            
            gui::TableNextColumn();
            {
                draw_image(dm_fb.color);
                gui::TableNextColumn();
                gui::Text("Dijkstra Map");
                bool changed = false;
                static bool colors = false;
                static bool inverse = false;
                changed |= gui::Checkbox("Use Colors##DM", &colors);
                changed |= gui::Checkbox("Invert Colors##DM", &inverse);
                gui::Text("Source values");
                ForRange(i, 0, dm_values.size()) {                    
                    gui::PushID(i);
                    changed |= gui::SliderInt("Value##DM", &dm_values[i], -100, 100);
                    gui::PopID();
                }
                if(changed) {
                    make_dijkstra_map(dm, dm_values);
                    draw_dijkstra_map(dm_fb, dm, basic_shader, colors, inverse);
                }
            }
            gui::TableNextColumn();
            {
                draw_image(vr_fb.color);
                gui::TableNextColumn();
                gui::Text("Voronoi");
                bool changed = false;
                bool redraw = false;
                static int points = 500;
                static int relaxations = 5;
                static bool connections = false;
                static String seed_buffer = make_string_buffer(128, "0");
                auto seed = atoi(&seed_buffer[0]);
                gui::InputTextWithHint("Seed (0 ^= random)##VR", seed_hint, &seed_buffer[0], seed_buffer.size(), ImGuiInputTextFlags_CharsDecimal); 
                // @todo Implement a custom allocator malloc struggles with rapid large allocations
                // jc_voronoi.h jcv_alloc/free_fn
                gui::SliderInt("Points##VR", &points, 100, 1000);
                gui::SliderInt("Relaxations##VR", &relaxations, 1, 20);

                redraw |= gui::Checkbox("Draw Connections##VR", &connections);
                changed |= gui::Button("Generate##VR");
                if(changed) {
                    vr.destroy();
                    make_voronoi(vr, seed, points, relaxations);
                    draw_voronoi(vr_fb, vr, basic_shader, connections);
                } else if(redraw) {
                    draw_voronoi(vr_fb, vr, basic_shader, connections);
                }
            }
            gui::TableNextColumn();
            {
                draw_image(ca_fb.color);
                gui::TableNextColumn();
                gui::Text("Cellular Automata");
                static String seed_buffer = make_string_buffer(128, "1");
                auto seed = atoi(&seed_buffer[0]);
                gui::InputTextWithHint("Seed##CA", seed_hint, &seed_buffer[0], seed_buffer.size(), ImGuiInputTextFlags_CharsDecimal); 
                static int iteration = 5;
                bool changed = false;
                changed |= gui::SliderInt("Iterations##CA", &iteration, 0, 100);
                // changed |= gui::Button("Generate##CA");
                if(changed) {
                    make_cellular_automata(ca, seed, iteration);
                    draw_cellular_automata(ca_fb, ca, basic_shader);
                }
            }   
            gui::TableNextColumn();
            {                
                draw_image(srp_fb.color);
                gui::TableNextColumn();
                gui::Text("Simple Room Placement");
                static int max_paths = 20;
                static String seed_buffer(128, '\0');
                auto seed = atoi(&seed_buffer[0]);   
                gui::InputTextWithHint("Seed##SRP", seed_hint, &seed_buffer[0], seed_buffer.size(), ImGuiInputTextFlags_CharsDecimal); 
                bool changed = false;
                // bool changed = gui::Button("Generate##SRP");
                changed |= gui::SliderInt("Max Paths##SRP", &max_paths, 0, 30);
                if(changed) {
                    srp = make_simple_room_placement(seed, max_paths);
                    draw_srp(srp_fb, srp, basic_shader);                    
                }
            }
            gui::TableNextColumn();
            {
                draw_image(rw_image);
                gui::TableNextColumn();
                gui::Text("Random Walk");
                // static Buffer<char, 128> seed_buffer = {};
                static String seed_buffer(128, '\0');
                auto seed = atoi(&seed_buffer[0]);   
                gui::InputTextWithHint("Seed##RW", seed_hint, &seed_buffer[0], seed_buffer.size(), ImGuiInputTextFlags_CharsDecimal); 
                static int iteration = 25;
                static float step_scale = 0.1;
                gui::SliderInt("Iterations##RW", &iteration, 1, 400);
                gui::SliderFloat("Step Scale##RW", &step_scale, 0.0001, 0.1);
                gui::SetItemTooltip("Steps taken: scale * area");
                bool changed = gui::Button("Generate##RW");
                gui::SetItemTooltip("Generate");                

                if(changed) {
                    destroy_texture(rw_image);
                    rw_image = random_walk_and_load(iteration, step_scale, seed);
                }
            }            
            gui::TableNextColumn();
            {
                draw_image(noise_image);
                gui::TableNextColumn();
                gui::Text("Noise Generator");
                bool changed = false;
                

                static String seed_buffer = make_string_buffer(128, "1");
                noise.seed = atoi(&seed_buffer[0]);                
                changed |= gui::InputTextWithHint("Seed##NS", seed_hint, &seed_buffer[0], seed_buffer.size(), ImGuiInputTextFlags_CharsDecimal);
                changed |= gui::SliderFloat("Frequency##NS", &noise.freq, 0.0, 1.0);
                changed |= gui::SliderFloat("Gain##NS", &noise.gain, 0.0, 1.0);
                changed |= gui::SliderInt("Octaves##NS", &noise.octaves, 0, 10);
                changed |= gui::SliderFloat("Lacunarity##NS", &noise.lacunarity, 0.01, 2.0);                
                // Can be -1 if undecided
                static int noise_type = 0;
                if(gui::Combo("Noise Type##NS", &noise_type, "OpenSimplex2\0OpenSimplex2S\0Cellular\0Perlin\0Value\0ValueCubic\0")) {
                    using enum FastNoiseLite::NoiseType;
                    switch(noise_type) {
                        case 0: noise.noise_type = NoiseType_OpenSimplex2; break;
                        case 1: noise.noise_type = NoiseType_OpenSimplex2S; break;
                        case 2: noise.noise_type = NoiseType_Cellular; break;
                        case 3: noise.noise_type = NoiseType_Perlin; break;
                        case 4: noise.noise_type = NoiseType_Value; break;
                        case 5: noise.noise_type = NoiseType_ValueCubic; break;
                    }
                    changed = true;
                }
                static int fractal_type = 0;
                if(gui::Combo("Fractal Type##NS", &fractal_type, "Fbm\0Ridged\0PingPong\0DomainWarpProgressive\0DomainWarpIndependent\0None\0")) {
                    using enum FastNoiseLite::FractalType;
                    switch(fractal_type) {
                        case 0: noise.fractal_type = FractalType_FBm; break;
                        case 1: noise.fractal_type = FractalType_Ridged; break;
                        case 2: noise.fractal_type = FractalType_PingPong; break;
                        case 3: noise.fractal_type = FractalType_DomainWarpProgressive; break;
                        case 4: noise.fractal_type = FractalType_DomainWarpIndependent; break;
                        case 5: noise.fractal_type = FractalType_None; break;
                    }
                    changed = true;
                }
                static bool colors = true;
                changed |= gui::Checkbox("Use Colors##NS", &colors);
                ForRange(i, 0, noise.limits.size()) {                    
                    gui::PushID(i);
                    changed |= gui::SliderFloat("Value##NS", &noise.limits[i], 0.0, 1.);
                    gui::PopID();
                }
                
                if(changed) {
                    noise.update();
                    destroy_texture(noise_image);                    
                    noise_image = make_noise_image(noise, colors);
                }
            }   
            gui::EndTable();     
        }
        
                
        gui::End();            
        gui::Render();
        // GLuint fb = 0;
        // glGenFramebuffers(1, &fb);
        // glBindFramebuffer(GL_FRAMEBUFFER, fb);
    
        int display_w, display_h;

        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(gui::GetDrawData());

        // use_main_framebuffer();
        // glfwGetFramebufferSize(window, &display_w, &display_h);
        // glViewport(0, 0, display_w, display_h);
        // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        // glClear(GL_COLOR_BUFFER_BIT);
        // use_shader(fb_shader);
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, tbuffer.color.internal);
        // auto ro = make_render_object();
        // defer(destroy(ro));
        // draw(ro);

        glfwSwapBuffers(window);
    }
    assets_destroy();
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    gui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

GLFWwindow *init_glfw_and_imgui() {

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return nullptr;


#if defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif


    GLFWwindow *window = glfwCreateWindow(1920, 1080, "Algorithms", nullptr, nullptr);
    if (window == nullptr)
        return nullptr;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    gui::CreateContext();
    ImGuiIO& io = gui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    gui::StyleColorsDark();
    // gui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window, true);

    ImGui_ImplOpenGL3_Init(glsl_version);

    return window;
}


void test_draw(Framebuffer &fb, Shader *shader) {
    fb.modulate = {0,0,0,1};
    use_framebuffer(fb);
    
    // Array<Vec3> pos = {};
    // Array<u32> ind = {};
    // const f32 D = 0.0;
    // push_tri(pos, ind, {-1,-1+D,0}, {0,1+D,0}, {1,-1+D,0});
    // auto ro =  make_render_object(pos, ind);
    // glUseProgram(0);
    // draw(ro)
    draw_texture(fb, {0,0}, {1,1}, Color::White, shader);
    
    // draw_texture(ro, fb, Vec2(0.5, 0.5), {1,1}, rand_rgb(), shader);
}