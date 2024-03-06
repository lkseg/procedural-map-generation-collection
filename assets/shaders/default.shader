
Custom File format

#VERTEX

layout (location = 0) in vec3 l_pos;
layout (location = 1) in vec3 l_color;
layout (location = 2) in vec2 l_uv;

uniform vec3 u_scale = vec3(1., 1., 1.);
uniform mat4 u_model;
uniform vec3 u_translation;


out vec3 v_color;
out vec2 v_uv;

void main() {
    vec4 pos = u_model * vec4(l_pos*u_scale, 1);
    vec3 p = pos.xyz + u_translation;
    p.z = 0;
    gl_Position = vec4(p, 1.0);


    v_color = l_color;
    v_uv = l_uv;
}

#FRAGMENT
out vec4 f_color;

in vec3 v_color; 
in vec2 v_uv; 

uniform sampler2D texture0;
uniform vec4 u_color = vec4(1.,1.,1.,1.);

void main() {
    f_color = u_color * texture(texture0, v_uv);
    // f_color = vec4(v_color, 1);
}

