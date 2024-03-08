#include "renderer.h"


Gl_State gl_state = {};

void init_renderer() {
    gl_state = {};

    glFrontFace(GL_CW);    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);    
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK); 
}



void destroy(const Render_Object &ro) {
    glDeleteVertexArrays(1, &ro.internal);
    glDeleteBuffers(1, &ro.vbo);
    glDeleteBuffers(1, &ro.ebo);
}

void use_shader(Shader *shader) {
    gl_state.shader = shader->internal;
    glUseProgram(shader->internal);
}

void draw(const Render_Object &ro) {
    glBindVertexArray(ro.internal); 
    glDrawElements(GL_TRIANGLES, ro.indices_count, GL_UNSIGNED_INT, nullptr);
}

Render_Object make_render_object() {
    constexpr f32 F = 0.5;
    Vertex vertices[] = {       
        Vertex{{-F,  F, 0.0},   {1.0, 1.0, 0.0},    {0.0, 1.0}},
        Vertex{{ F,  F, 0.0},   {1.0, 0.0, 0.0},    {1.0, 1.0}},
        Vertex{{-F, -F, 0.0},   {0.0, 0.0, 1.0},    {0.0, 0.0}},
        Vertex{{ F, -F, 0.0},   {0.0, 1.0, 0.0},    {1.0, 0.0}},        
        
    };
    u32 indices[] = {
        0, 1, 2,
        1, 3, 2,
    };
    // u32 indices[] = {
    //     0, 2, 1,
    //     1, 2, 3,
    // };
    u32 VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    constexpr u32 STRIDE = sizeof(Vertex);
    glVertexAttribPointer(0, sizeof(Vec3)/sizeof(f32), GL_FLOAT, GL_FALSE, STRIDE, (void *)offsetof(Vertex, position));        
    glVertexAttribPointer(1, sizeof(Vec3)/sizeof(f32), GL_FLOAT, GL_FALSE, STRIDE, (void *)offsetof(Vertex, color));        
    glVertexAttribPointer(2, sizeof(Vec2)/sizeof(f32), GL_FLOAT, GL_FALSE, STRIDE, (void *)offsetof(Vertex, uv));    
    
    // Just remove?
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    return {.internal = VAO, .ebo = EBO, .vbo = VBO, .indices_count = 6};
}

void make_line(Buffer<Vec3, 6> &buffer, Vec2 a, Vec2 b, f32 width) {
    Vec2 d = b - a;
    Vec2 r = normalize(rotate(d, 0.25))*width*0.5f;
    Vec3 x0 = vec3(b-r), x1 = vec3(b+r), x2 = vec3(a-r), x3 = vec3(a+r);
    buffer = Buffer<Vec3, 6>({x0, x1, x2,   x1, x3, x2});
}

void push_line(Array<Vec3> &vert, Array<u32> &ind, Vec2 a, Vec2 b, f32 width) {
    Buffer<Vec3, 6> bf;
    make_line(bf, a, b, width);
    // CW
    push_tri(vert, ind, bf[2], bf[1], bf[0]);
    push_tri(vert, ind, bf[5], bf[4], bf[3]);
}
void push_line(Basic_Mesh &mesh, Vec2 a, Vec2 b, f32 width) {
    push_line(mesh.vertices, mesh.indices, a, b, width);
}

Render_Object make_render_object(Array<Vec3> &positions, Array<u32> &indices) {
    u32 VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.bytes_size(), &indices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, positions.bytes_size(), &positions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    constexpr u32 STRIDE = sizeof(Vec3);
    glVertexAttribPointer(0, sizeof(Vec3)/sizeof(f32), GL_FLOAT, GL_FALSE, STRIDE, (void *)0);    
    return Render_Object{.internal = VAO, .ebo = EBO, .vbo = VBO, .indices_count = (u32)indices.size()};
}

void push_tri(Array<Vec3> &positions, Array<u32> &indices, Vec3 a, Vec3 b, Vec3 c) {
    positions.push_back(a);
    positions.push_back(b);
    positions.push_back(c);
    const Buffer<u32, 3> ind_base = { 0, 1, 2};
    u32 offset = indices.size();
    for(auto &ind: ind_base) {
        indices.push_back(offset + ind);
    }
}
Framebuffer make_framebuffer() {
    GLuint fb = 0;
    // @todo?
    // defer(glDeleteFramebuffers(1, &fb));

    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    return {fb};
}
Framebuffer make_framebuffer_target(Vec2i size) {
    auto fb = make_framebuffer();
    
    glBindFramebuffer(GL_FRAMEBUFFER, fb.internal);

    GLuint target;
    glGenTextures(1, &target);
    
    glBindTexture(GL_TEXTURE_2D, target);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
    
    GLuint rbo;
    // @todo?
    // defer(glDeleteRenderbuffers(1, &rbo));
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y); 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
    // glBindFramebuffer(GL_FRAMEBUFFER, fb);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) panic("Bad gl init");
    fb.has_target = true;
    fb.color = Texture{.internal = target, .size = {size.x, size.y}};
    fb.rbo = rbo;
    // draw buffers? @todo
    // auto buffers = to_buffer<GLenum, 1>({GL_COLOR_ATTACHMENT0});
    // glDrawBuffers(buffers.size(), &buffers[0]); 
    return fb;
}

void use_main_framebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gl_state.framebuffer = 0;
}
void use_framebuffer(const Framebuffer &fb) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb.internal);
    gl_state.framebuffer = fb.internal;
    glClearColor(fb.modulate.x, fb.modulate.y, fb.modulate.z, fb.modulate.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(fb.has_target) {
        glViewport(0, 0, fb.color.size.x, fb.color.size.y);
    }
}

void destroy(const Framebuffer &fb) {
    if(fb.has_target) {
        glDeleteRenderbuffers(1, &fb.rbo);
        glDeleteTextures(1, &fb.color.internal);
    }
    glDeleteFramebuffers(1, &fb.internal);
}
Texture draw_texture(Framebuffer fb, Vec2 position, Vec2 scale, Color color, Shader *shader, const Texture *texture) {
    auto ro = make_render_object();    
    defer(destroy(ro));    
    return draw_texture(ro, fb, position, scale, color, shader, texture);
}
Texture draw_texture(Render_Object &ro, Framebuffer fb, Vec2 position, Vec2 scale, Color color, Shader *shader, const Texture *texture) {
    assert(fb.has_target);
    assert(gl_state.framebuffer == fb.internal, "Framebuffer not set");
    // glBindFramebuffer(GL_FRAMEBUFFER, fb.internal);		
    // glDisable(GL_DEPTH_TEST);
        
    use_shader(shader);
    if(texture) {
        glBindTexture(GL_TEXTURE_2D, texture->internal);    
    }
    // We use [-0.5,0.5] while opengl wants [-1, 1] for vertex positions so multiply by 2
    set_uniform(shader, "u_scale", Vec3{2.*scale.x, 2.*scale.y, 0.});    
    
    // Y position is flipped when drawing an image through dearimgui since it expects it to be flipped from Opengls view.
    // This ultimately really doesn't matter for the showcase.
    Mat4 model = Mat4({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1});    
    model.w = {-1,-1,0,0};
    // Mat3 model = Mat3({0,0,0}, {0,0,0}, {0,0,0});
    set_uniform(shader, "u_model", model);
    Vec3 translation = Vec3(position, 0.0);
    // Default 2d coordinates i.e.
    // (0,0) ------ (1,0)
    //  |
    //  |
    //  (0,1)   

    // print(typeid(2 * translation).name());
    set_uniform(shader, "u_translation", Vec3(2. * translation));
    set_uniform(shader, "u_color", color_vec4(color));
    // glUseProgram(0);
    draw(ro);	
    return fb.color;
}

Vec3 color_vec3(Color c) {
    return Vec3{f32(c.r)/f32(255),f32(c.g)/f32(255),f32(c.b)/f32(255)};
}
Vec4 color_vec4(Color c) {
    return Vec4{f32(c.r)/f32(255),f32(c.g)/f32(255),f32(c.b)/f32(255),f32(c.a)/f32(255)};
}