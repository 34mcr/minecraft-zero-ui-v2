#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_tex_coord;
layout (location = 2) in float a_tex_index;
layout (location = 3) in float a_brightness;

out vec2 v_tex_coord;
out float v_tex_index;
out float v_brightness;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(a_position, 1.0);
    v_tex_coord = a_tex_coord;
    v_tex_index = a_tex_index;
    v_brightness = a_brightness;
}
