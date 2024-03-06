#pragma once

#include "image.h"
#include "linalg.h"
#include <GLES3/gl3.h>
#include "buffer.h"

typedef GLuint ishader;

struct Shader {
    u64 stamp;
    String name;
    ishader internal;
};

struct Framebuffer {
	u32 internal = 0;
    bool has_target = false;
    Texture color;
    u32 rbo;
    Vec4 modulate = {1,1,1,1};
};
struct Vertex {
    Vec3 position;
    Vec3 color;
    Vec2 uv;
};
struct Render_Object {
	u32 internal = 0; // vao
	u32 ebo, vbo;    
    u32 indices_count;
};

struct Gl_State {
    ishader shader;
    u32 framebuffer;
};

extern Gl_State gl_state;

void init_renderer();
void use_main_framebuffer();
void use_framebuffer(const Framebuffer &);

Framebuffer make_framebuffer();
Framebuffer make_framebuffer_target(Vec2i = Vec2i{500, 500});

Render_Object make_render_object();
Render_Object make_render_object(Array<Vec3> &, Array<u32> &);

void push_line(Array<Vec3> &, Array<u32> &, Vec2 a, Vec2 b, f32 width);
void make_line(Buffer<Vec3, 6> &buffer, Vec2 a, Vec2 b, f32 width);

// Simple drawing 'api' thats draws on the framebuffer's color attachment.
// *position* is in uv coordinates and (0, 0) is the upper left corner.
// Draws a rectangle
Texture draw_texture(Framebuffer, Vec2 position, Vec2 scale, Color, Shader *, const Texture * = nullptr);
// Draws the custom Render_Object
Texture draw_texture(Render_Object &, Framebuffer, Vec2 position, Vec2 scale, Color, Shader *, const Texture * = nullptr);

void push_tri(Array<Vec3> &positions, Array<u32> &indices, Vec3 a, Vec3 b, Vec3 c);

void use_shader(Shader *);
void draw(const Render_Object &);
void destroy(const Render_Object &);
void destroy(const Framebuffer &);

template<typename T>
void set_uniform(Shader *shader, const String &name, const T &v) {
    // Have to use it to set shaders
    // use_shader(shader);
    assert(gl_state.shader == shader->internal, "Shader is not active");

    auto loc = glGetUniformLocation(shader->internal, name.c_str());
    #define if_type(A_Type) if constexpr(std::is_same<T, A_Type>::value)

    if_type(Vec3) {
        glUniform3f(loc, v.x, v.y, v.z);
    } else if_type(Vec4) {
        glUniform4f(loc, v.x, v.y, v.z, v.w);
    } else if_type(Mat3) {
        glUniformMatrix3fv(loc, 1, GL_FALSE, &v[0][0]);
    }  else if_type(Mat4) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, &v[0][0]);
    } else {
        panic("Bad Uniform @todo");
    }
    #undef if_type
}

Vec3 color_vec3(Color);
Vec4 color_vec4(Color);

