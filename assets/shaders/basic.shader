Custom File format

#VERTEX

layout (location = 0) in vec3 l_pos;

uniform vec3 u_scale = vec3(1., 1., 1.);
uniform mat4 u_model;
uniform vec3 u_translation;

void main() {
    vec4 pos = u_model * vec4(l_pos*u_scale, 1);
    vec3 p = pos.xyz + u_translation;
    p.z = 0;
    gl_Position = vec4(p, 1.0);
}

#FRAGMENT
out vec4 f_color;
uniform vec4 u_color = vec4(1,1,1,1);

void main() {
    f_color = u_color * vec4(1,1,1,1);
}
