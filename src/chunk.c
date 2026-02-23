#include "chunk.h"
#include "block.h"
#include "math/vec3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

// 方块顶点数据（立方体的24个顶点）
static const float CUBE_VERTICES[] = {
    // 位置              // 法线              // 纹理坐标
    // 前面 (Z+)
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
    
    // 后面 (Z-)
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    
    // 左面 (X-)
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    
    // 右面 (X+)
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    
    // 上面 (Y+)
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    
    // 下面 (Y-)
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f
};

// 立方体索引
static const unsigned int CUBE_INDICES[] = {
    0, 1, 2, 2, 3, 0,     // 前面
    4, 5, 6, 6, 7, 4,     // 后面
    8, 9, 10, 10, 11, 8,  // 左面
    12, 13, 14, 14, 15, 12, // 右面
    16, 17, 18, 18, 19, 16, // 上面
    20, 21, 22, 22, 23, 20  // 下面
};

// 创建区块
Chunk chunk_create(int x, int z) {
    Chunk chunk = {0};
    chunk.x = x;
    chunk.z = z;
    chunk.vertex_count = 0;
    chunk.needs_rebuild = true;
    chunk.is_empty = true;
    
    // 初始化所有方块为空气
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                chunk.blocks[x][y][z] = block_create(BLOCK_AIR);
            }
        }
    }
    
    // 创建OpenGL对象
    glGenVertexArrays(1, &chunk.vao);
    glGenBuffers(1, &chunk.vbo);
    
    printf("Chunk created at (%d, %d)\\n", x, z);
    return chunk;
}

// 销毁区块
void chunk_destroy(Chunk* chunk) {
    if (chunk->vao) {
        glDeleteVertexArrays(1, &chunk->vao);
    }
    if (chunk->vbo) {
        glDeleteBuffers(1, &chunk->vbo);
    }
    
    chunk->vao = 0;
    chunk->vbo = 0;
    chunk->vertex_count = 0;
}

// 设置方块
void chunk_set_block(Chunk* chunk, int x, int y, int z, BlockType type) {
    if (x < 0 || x >= CHUNK_SIZE || 
        y < 0 || y >= CHUNK_HEIGHT || 
        z < 0 || z >= CHUNK_SIZE) {
        return;
    }
    
    chunk->blocks[x][y][z] = block_create(type);
    chunk->needs_rebuild = true;
    
    // 更新是否为空
    if (type != BLOCK_AIR) {
        chunk->is_empty = false;
    }
}

// 获取方块
BlockType chunk_get_block(const Chunk* chunk, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || 
        y < 0 || y >= CHUNK_HEIGHT || 
        z < 0 || z >= CHUNK_SIZE) {
        return BLOCK_AIR;
    }
    
    return chunk->blocks[x][y][z].type;
}

// 检查方块是否可见
static bool chunk_is_face_visible(const Chunk* chunk, int x, int y, int z, int face) {
    // 计算相邻方块坐标
    int nx = x, ny = y, nz = z;
    
    switch (face) {
        case 0: nx--; break; // 西面
        case 1: nx++; break; // 东面
        case 2: ny--; break; // 下面
        case 3: ny++; break; // 上面
        case 4: nz--; break; // 北面
        case 5: nz++; break; // 南面
    }
    
    // 检查边界
    if (nx < 0 || nx >= CHUNK_SIZE || nz < 0 || nz >= CHUNK_SIZE) {
        return true; // 区块边界，需要渲染
    }
    
    if (ny < 0 || ny >= CHUNK_HEIGHT) {
        return (face == 2 && y == 0) || (face == 3 && y == CHUNK_HEIGHT - 1);
    }
    
    // 获取相邻方块
    BlockType neighbor = chunk->blocks[nx][ny][nz].type;
    
    // 如果相邻方块是空气或透明，则需要渲染这个面
    if (neighbor == BLOCK_AIR) {
        return true;
    }
    
    const BlockInfo* info = block_get_info(neighbor);
    if (info && info->transparent && neighbor != chunk->blocks[x][y][z].type) {
        return true;
    }
    
    return false;
}

// 获取面的亮度
static float chunk_get_face_brightness(int face) {
    switch (face) {
        case 0: return 0.6f; // 西面
        case 1: return 0.7f; // 东面
        case 2: return 0.5f; // 下面
        case 3: return 0.9f; // 上面
        case 4: return 0.6f; // 北面
        case 5: return 0.8f; // 南面
        default: return 1.0f;
    }
}

// 添加方块的一个面到网格
static void chunk_add_face(Chunk* chunk, 
                          float* vertices, int* vertex_index,
                          int x, int y, int z, 
                          int face, BlockType type, float brightness) {
    // 获取面的纹理索引
    int texture_index = block_get_texture(type, face);
    
    // 计算方块在世界中的位置
    float world_x = chunk->x * CHUNK_SIZE + x;
    float world_y = y;
    float world_z = chunk->z * CHUNK_SIZE + z;
    
    // 面的起始顶点索引
    int face_start = face * 4;
    
    // 添加4个顶点
    for (int i = 0; i < 4; i++) {
        int vertex_offset = face_start + i;
        
        // 位置
        float pos_x = CUBE_VERTICES[vertex_offset * 8 + 0] + world_x;
        float pos_y = CUBE_VERTICES[vertex_offset * 8 + 1] + world_y;
        float pos_z = CUBE_VERTICES[vertex_offset * 8 + 2] + world_z;
        
        // 法线
        float normal_x = CUBE_VERTICES[vertex_offset * 8 + 3];
        float normal_y = CUBE_VERTICES[vertex_offset * 8 + 4];
        float normal_z = CUBE_VERTICES[vertex_offset * 8 + 5];
        
        // 纹理坐标
        float tex_u = CUBE_VERTICES[vertex_offset * 8 + 6];
        float tex_v = CUBE_VERTICES[vertex_offset * 8 + 7];
        
        // 填充顶点数据
        vertices[(*vertex_index)++] = pos_x;
        vertices[(*vertex_index)++] = pos_y;
        vertices[(*vertex_index)++] = pos_z;
        
        vertices[(*vertex_index)++] = normal_x;
        vertices[(*vertex_index)++] = normal_y;
        vertices[(*vertex_index)++] = normal_z;
        
        vertices[(*vertex_index)++] = tex_u;
        vertices[(*vertex_index)++] = tex_v;
        
        vertices[(*vertex_index)++] = (float)texture_index;
        vertices[(*vertex_index)++] = brightness;
    }
}

// 重建区块网格
void chunk_rebuild(Chunk* chunk) {
    if (!chunk->needs_rebuild) {
        return;
    }
    
    // 临时顶点缓冲区
    float vertices[CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE * 24 * 10]; // 最多24个面 * 10个属性
    int vertex_index = 0;
    
    // 检查区块是否为空
    chunk->is_empty = true;
    
    // 遍历所有方块
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                BlockType type = chunk->blocks[x][y][z].type;
                
                if (type == BLOCK_AIR) {
                    continue;
                }
                
                chunk->is_empty = false;
                
                const BlockInfo* info = block_get_info(type);
                if (!info || !info->solid) {
                    continue;
                }
                
                // 检查6个面
                for (int face = 0; face < 6; face++) {
                    if (chunk_is_face_visible(chunk, x, y, z, face)) {
                        float brightness = chunk_get_face_brightness(face);
                        chunk_add_face(chunk, vertices, &vertex_index, 
                                      x, y, z, face, type, brightness);
                    }
                }
            }
        }
    }
    
    // 如果区块为空，清理并返回
    if (chunk->is_empty) {
        glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
        chunk->vertex_count = 0;
        chunk->needs_rebuild = false;
        return;
    }
    
    // 上传顶点数据到GPU
    glBindVertexArray(chunk->vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
    
    // 分配缓冲区
    glBufferData(GL_ARRAY_BUFFER, vertex_index * sizeof(float), 
                 vertices, GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 法线属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), 
                         (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 纹理坐标属性
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), 
                         (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // 纹理索引属性
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), 
                         (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // 亮度属性
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), 
                         (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(4);
    
    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    chunk->vertex_count = vertex_index / 10; // 每个顶点10个属性
    chunk->needs_rebuild = false;
    
    printf("Chunk (%d, %d) rebuilt: %d vertices\\n", 
           chunk->x, chunk->z, chunk->vertex_count);
}

// 渲染区块
void chunk_render(const Chunk* chunk) {
    if (chunk->is_empty || chunk->vertex_count == 0) {
        return;
    }
    
    glBindVertexArray(chunk->vao);
    glDrawArrays(GL_TRIANGLES, 0, chunk->vertex_count);
    glBindVertexArray(0);
}

// 检查区块是否为空
bool chunk_is_empty(const Chunk* chunk) {
    return chunk->is_empty;
}

// 获取区块坐标
void chunk_get_position(const Chunk* chunk, int* x, int* z) {
    if (x) *x = chunk->x;
    if (z) *z = chunk->z;
}

// 获取区块中的方块数量
int chunk_get_block_count(const Chunk* chunk) {
    int count = 0;
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                if (chunk->blocks[x][y][z].type != BLOCK_AIR) {
                    count++;
                }
            }
        }
    }
    return count;
}
