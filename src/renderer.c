// renderer.c
#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VERTICES 65536
#define MAX_INDICES 131072

static Renderer g_renderer;

// 简单的顶点和片段着色器源码
static const char* VERTEX_SHADER_SRC = 
    "#version 330 core\\n"
    "layout(location = 0) in vec3 aPos;\\n"
    "layout(location = 1) in vec2 aTexCoord;\\n"
    "layout(location = 2) in vec4 aColor;\\n"
    "out vec2 TexCoord;\\n"
    "out vec4 Color;\\n"
    "uniform mat4 projection;\\n"
    "uniform mat4 view;\\n"
    "uniform mat4 model;\\n"
    "void main() {\\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\\n"
    "    TexCoord = aTexCoord;\\n"
    "    Color = aColor;\\n"
    "}\\n";

static const char* FRAGMENT_SHADER_SRC =
    "#version 330 core\\n"
    "in vec2 TexCoord;\\n"
    "in vec4 Color;\\n"
    "out vec4 FragColor;\\n"
    "uniform sampler2D texture1;\\n"
    "void main() {\\n"
    "    FragColor = texture(texture1, TexCoord) * Color;\\n"
    "    if (FragColor.a < 0.1) discard;\\n"  // 透明像素丢弃
    "}\\n";

// 编译着色器
static GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // 检查错误
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
GLuint create_shader_program(void) {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, VERTEX_SHADER_SRC);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SRC);
    
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
    
    return program;
}

// 初始化渲染器
void renderer_init(int width, int height) {
    printf("初始化渲染器...\\n");
    
    // 初始化 GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW 初始化失败: %s\\n", glewGetErrorString(err));
        return;
    }
    
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // 启用面剔除
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // 启用混合（透明度）
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 清屏颜色
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // 天空蓝
    
    // 创建着色器
    g_renderer.shader_program = create_shader_program();
    glUseProgram(g_renderer.shader_program);
    
    // 设置投影矩阵
    float aspect = (float)width / (float)height;
    mat4_perspective(g_renderer.projection, 45.0f, aspect, 0.1f, 100.0f);
    
    // 初始化模型矩阵
    mat4_identity(g_renderer.model);
    
    // 初始化顶点数组
    glGenVertexArrays(1, &g_renderer.batch.vao);
    glGenBuffers(1, &g_renderer.batch.vbo);
    glGenBuffers(1, &g_renderer.batch.ebo);
    
    glBindVertexArray(g_renderer.batch.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_renderer.batch.vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
    
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 颜色属性
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // 索引缓冲
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_renderer.batch.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(unsigned int), NULL, GL_DYNAMIC_DRAW);
    
    // 分配内存
    g_renderer.batch.vertices = malloc(MAX_VERTICES * sizeof(Vertex));
    g_renderer.batch.indices = malloc(MAX_INDICES * sizeof(unsigned int));
    g_renderer.batch.vertex_count = 0;
    g_renderer.batch.index_count = 0;
    
    printf("渲染器初始化完成\\n");
}

// 清理资源
void renderer_cleanup(void) {
    printf("清理渲染器...\\n");
    
    glDeleteVertexArrays(1, &g_renderer.batch.vao);
    glDeleteBuffers(1, &g_renderer.batch.vbo);
    glDeleteBuffers(1, &g_renderer.batch.ebo);
    glDeleteProgram(g_renderer.shader_program);
    
    free(g_renderer.batch.vertices);
    free(g_renderer.batch.indices);
    
    g_renderer.batch.vertex_count = 0;
    g_renderer.batch.index_count = 0;
}

// 开始一帧
void renderer_begin_frame(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(g_renderer.shader_program);
    
    // 重置批次
    g_renderer.batch.vertex_count = 0;
    g_renderer.batch.index_count = 0;
}

// 结束一帧
void renderer_end_frame(void) {
    if (g_renderer.batch.vertex_count > 0) {
        glBindVertexArray(g_renderer.batch.vao);
        
        // 上传顶点数据
        glBindBuffer(GL_ARRAY_BUFFER, g_renderer.batch.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                       g_renderer.batch.vertex_count * sizeof(Vertex),
                       g_renderer.batch.vertices);
        
        // 上传索引数据
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_renderer.batch.ebo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                       g_renderer.batch.index_count * sizeof(unsigned int),
                       g_renderer.batch.indices);
        
        // 绘制
        glDrawElements(GL_TRIANGLES, g_renderer.batch.index_count, 
                      GL_UNSIGNED_INT, 0);
    }
}

// 绘制立方体
void renderer_draw_cube(vec3 position, vec3 size, vec4 color) {
    // 简单的立方体绘制（可以优化）
    // 这里实现6个面的绘制
    float x = position.x, y = position.y, z = position.z;
    float w = size.x, h = size.y, d = size.z;
    
    // 8个顶点
    Vertex vertices[] = {
        // 前面
        {x, y, z, 0, 0, color.r, color.g, color.b, color.a},
        {x+w, y, z, 1, 0, color.r, color.g, color.b, color.a},
        {x+w, y+h, z, 1, 1, color.r, color.g, color.b, color.a},
        {x, y+h, z, 0, 1, color.r, color.g, color.b, color.a},
        // 后面
        {x, y, z+d, 0, 0, color.r, color.g, color.b, color.a},
        {x+w, y, z+d, 1, 0, color.r, color.g, color.b, color.a},
        {x+w, y+h, z+d, 1, 1, color.r, color.g, color.b, color.a},
        {x, y+h, z+d, 0, 1, color.r, color.g, color.b, color.a},
    };
    
    // 6个面的索引
    unsigned int indices[] = {
        // 前面
        0, 1, 2, 2, 3, 0,
        // 后面
        4, 5, 6, 6, 7, 4,
        // 左面
        4, 0, 3, 3, 7, 4,
        // 右面
        1, 5, 6, 6, 2, 1,
        // 上面
        3, 2, 6, 6, 7, 3,
        // 下面
        0, 1, 5, 5, 4, 0
    };
    
    // 添加到批次
    int base_vertex = g_renderer.batch.vertex_count;
    for (int i = 0; i < 8; i++) {
        g_renderer.batch.vertices[g_renderer.batch.vertex_count++] = vertices[i];
    }
    
    for (int i = 0; i < 36; i++) {
        g_renderer.batch.indices[g_renderer.batch.index_count++] = indices[i] + base_vertex;
    }
}

// 设置摄像机
void renderer_set_camera(Camera* camera) {
    camera_get_view_matrix(camera, g_renderer.view);
    
    // 上传矩阵到着色器
    GLint proj_loc = glGetUniformLocation(g_renderer.shader_program, "projection");
    GLint view_loc = glGetUniformLocation(g_renderer.shader_program, "view");
    GLint model_loc = glGetUniformLocation(g_renderer.shader_program, "model");
    
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, &g_renderer.projection[0][0]);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, &g_renderer.view[0][0]);
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, &g_renderer.model[0][0]);
}
