#ifndef CHUNK_H
#define CHUNK_H

#include "block.h"
#include <stdbool.h>

#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 256

typedef struct {
    int x, z;  // 区块坐标
    Block blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
    
    // 渲染数据
    unsigned int vao;
    unsigned int vbo;
    int vertex_count;
    
    bool needs_rebuild;
    bool is_empty;
} Chunk;

Chunk chunk_create(int x, int z);
void chunk_destroy(Chunk* chunk);
void chunk_set_block(Chunk* chunk, int x, int y, int z, BlockType type);
BlockType chunk_get_block(const Chunk* chunk, int x, int y, int z);
void chunk_rebuild(Chunk* chunk);
void chunk_render(const Chunk* chunk);
bool chunk_is_visible(const Chunk* chunk, int face, int x, int y, int z);

#endif
