#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include "math/mat4.h"  // 包含数学库

typedef struct {
    GLuint id;
    const char* name;
} Shader;

// 着色器函数
Shader shader_create(const char* vertex_source, const char* fragment_source);
void shader_use(Shader shader);
void shader_delete(Shader shader);

// 统一变量设置
void shader_set_int(Shader shader, const char* name, int value);
void shader_set_float(Shader shader, const char* name, float value);
void shader_set_vec3(Shader shader, const char* name, vec3 value);
void shader_set_mat4(Shader shader, const char* name, mat4 value);

#endif
