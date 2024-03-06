#VERTEX

layout (location = 0) in vec3 l_pos;
layout (location = 1) in vec3 l_color;
layout (location = 2) in vec2 l_uv;

out vec3 v_color;
out vec2 v_uv;

void main() {
    gl_Position = vec4(l_pos, 1.0); 
    
    v_color = l_color;
    v_uv = l_uv;
}

#FRAGMENT
out vec4 f_color;

in vec3 v_color; 
in vec2 v_uv; 

uniform sampler2D texture0;

void main() {
    f_color = texture(texture0, v_uv);
}

