#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_WOOD,
    BLOCK_LEAVES,
    BLOCK_WATER,
    BLOCK_SAND,
    BLOCK_GLASS,
    BLOCK_COBBLESTONE,
    BLOCK_BRICK,
    BLOCK_TNT,
    BLOCK_BEDROCK,
    BLOCK_COUNT
} BlockType;

typedef struct {
    uint8_t type;
    uint8_t metadata;
    uint8_t light;
} Block;

typedef struct {
    const char* name;
    bool solid;
    bool transparent;
    bool liquid;
    float hardness;
    int texture_faces[6]; // 六个面的纹理索引
} BlockInfo;

Block block_create(BlockType type);
const BlockInfo* block_get_info(BlockType type);
bool block_is_solid(BlockType type);
bool block_is_transparent(BlockType type);
int block_get_texture(BlockType type, int face);

#endif
