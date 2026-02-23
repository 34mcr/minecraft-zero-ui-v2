#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 编译着色器
static GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // 检查编译错误
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "着色器编译错误:\\n%s\\n", info_log);
    }
    
    return shader;
}

// 创建着色器程序
Shader shader_create(const char* vertex_source, const char* fragment_source) {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    // 检查链接错误
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "着色器程序链接错误:\\n%s\\n", info_log);
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    Shader shader = {program, "default"};
    return shader;
}

// 使用着色器
void shader_use(Shader shader) {
    glUseProgram(shader.id);
}

// 删除着色器
void shader_delete(Shader shader) {
    glDeleteProgram(shader.id);
}

// 设置统一变量
void shader_set_int(Shader shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader.id, name), value);
}

void shader_set_float(Shader shader, const char* name, float value) {
    glUniform1f(glGetUniformLocation(shader.id, name), value);
}

void shader_set_vec3(Shader shader, const char* name, vec3 value) {
    glUniform3f(glGetUniformLocation(shader.id, name), value.x, value.y, value.z);
}

void shader_set_mat4(Shader shader, const char* name, mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(shader.id, name), 1, GL_FALSE, &value[0][0]);
}
