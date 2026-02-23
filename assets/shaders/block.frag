#version 330 core

in vec2 v_tex_coord;
in float v_tex_index;
in float v_brightness;
out vec4 frag_color;

uniform sampler2DArray texture_array;
uniform vec3 view_pos;
uniform float time;

void main() {
    // 从纹理数组采样
    vec4 color = texture(texture_array, vec3(v_tex_coord, v_tex_index));
    
    // 丢弃透明像素
    if (color.a < 0.1) {
        discard;
    }
    
    // 应用亮度
    color.rgb *= v_brightness;
    
    // 简单雾效
    float distance = length(view_pos);
    float fog_amount = 1.0 - exp(-distance * 0.005);
    vec3 fog_color = vec3(0.7, 0.8, 1.0);
    color.rgb = mix(color.rgb, fog_color, fog_amount);
    
    frag_color = color;
}
