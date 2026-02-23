#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "math/vec3.h"
#include <stdbool.h>

#define WORLD_SIZE 16  // 16x16区块

typedef struct {
    int seed;
    vec3 spawn_point;
    
    Chunk* chunks[WORLD_SIZE][WORLD_SIZE];
    int chunk_count;
    
    // 世界生成
    float (*noise)(float x, float z);
} World;

World world_create(int seed);
void world_destroy(World* world);
void world_generate(World* world);
void world_render(const World* world);
void world_update(World* world, vec3 player_pos);
BlockType world_get_block(const World* world, int x, int y, int z);
void world_set_block(World* world, int x, int y, int z, BlockType type);
vec3 world_get_spawn(const World* world);

#endif
