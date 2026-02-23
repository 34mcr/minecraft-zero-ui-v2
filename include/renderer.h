// renderer.h
#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include "math/mat4.h"
#include "camera.h"

// 方块顶点结构
typedef struct {
    float x, y, z;      // 位置
    float u, v;         // 纹理坐标
    float r, g, b, a;   // 颜色
} Vertex;

// 渲染批次
typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    Vertex* vertices;
    unsigned int* indices;
    int vertex_count;
    int index_count;
} RenderBatch;

// 渲染器
typedef struct {
    RenderBatch batch;
    mat4 projection;
    mat4 view;
    mat4 model;
    GLuint shader_program;
    GLuint texture;
} Renderer;

// 函数声明
void renderer_init(int width, int height);
void renderer_cleanup(void);
void renderer_begin_frame(void);
void renderer_end_frame(void);
void renderer_draw_cube(vec3 position, vec3 size, vec4 color);
void renderer_set_camera(Camera* camera);

// 简单着色器
GLuint create_shader_program(void);

#endif
