#include "block.h"
#include <string.h>

// 方块信息定义
static const BlockInfo g_block_info[] = {
    // BLOCK_AIR
    {.name = "Air", .solid = false, .transparent = true, .liquid = false, 
     .hardness = 0.0f, .texture_faces = {0, 0, 0, 0, 0, 0}},
    
    // BLOCK_GRASS
    {.name = "Grass", .solid = true, .transparent = false, .liquid = false,
     .hardness = 0.6f, .texture_faces = {0, 1, 2, 2, 2, 2}},  // 顶=草地, 底=泥土, 侧=草侧
    
    // BLOCK_DIRT
    {.name = "Dirt", .solid = true, .transparent = false, .liquid = false,
     .hardness = 0.5f, .texture_faces = {2, 2, 2, 2, 2, 2}},  // 全是泥土
    
    // BLOCK_STONE
    {.name = "Stone", .solid = true, .transparent = false, .liquid = false,
     .hardness = 1.5f, .texture_faces = {3, 3, 3, 3, 3, 3}},  // 全是石头
    
    // BLOCK_WOOD
    {.name = "Wood", .solid = true, .transparent = false, .liquid = false,
     .hardness = 2.0f, .texture_faces = {4, 4, 5, 5, 4, 4}},  // 顶底=木纹, 侧=木纹
    
    // BLOCK_LEAVES
    {.name = "Leaves", .solid = true, .transparent = true, .liquid = false,
     .hardness = 0.2f, .texture_faces = {6, 6, 6, 6, 6, 6}},  // 全是树叶
    
    // BLOCK_WATER
    {.name = "Water", .solid = false, .transparent = true, .liquid = true,
     .hardness = 0.0f, .texture_faces = {7, 7, 7, 7, 7, 7}},  // 全是水
    
    // BLOCK_SAND
    {.name = "Sand", .solid = true, .transparent = false, .liquid = false,
     .hardness = 0.5f, .texture_faces = {8, 8, 8, 8, 8, 8}},  // 全是沙子
    
    // BLOCK_GLASS
    {.name = "Glass", .solid = true, .transparent = true, .liquid = false,
     .hardness = 0.3f, .texture_faces = {9, 9, 9, 9, 9, 9}},  // 全是玻璃
    
    // BLOCK_COBBLESTONE
    {.name = "Cobblestone", .solid = true, .transparent = false, .liquid = false,
     .hardness = 2.0f, .texture_faces = {10, 10, 10, 10, 10, 10}},  // 全是圆石
    
    // BLOCK_BRICK
    {.name = "Brick", .solid = true, .transparent = false, .liquid = false,
     .hardness = 2.0f, .texture_faces = {11, 11, 11, 11, 11, 11}},  // 全是砖块
    
    // BLOCK_TNT
    {.name = "TNT", .solid = true, .transparent = false, .liquid = false,
     .hardness = 0.8f, .texture_faces = {12, 12, 13, 13, 12, 12}},  // 顶底=TNT顶, 侧=TNT侧
    
    // BLOCK_BEDROCK
    {.name = "Bedrock", .solid = true, .transparent = false, .liquid = false,
     .hardness = -1.0f, .texture_faces = {14, 14, 14, 14, 14, 14}},  // 全是基岩
};

Block block_create(BlockType type) {
    Block block = {0};
    block.type = type;
    block.light = 0;
    return block;
}

const BlockInfo* block_get_info(BlockType type) {
    if (type >= 0 && type < BLOCK_COUNT) {
        return &g_block_info[type];
    }
    return &g_block_info[BLOCK_AIR];
}

bool block_is_solid(BlockType type) {
    const BlockInfo* info = block_get_info(type);
    return info->solid;
}

bool block_is_transparent(BlockType type) {
    const BlockInfo* info = block_get_info(type);
    return info->transparent;
}

bool block_is_liquid(BlockType type) {
    const BlockInfo* info = block_get_info(type);
    return info->liquid;
}

float block_get_hardness(BlockType type) {
    const BlockInfo* info = block_get_info(type);
    return info->hardness;
}

int block_get_texture(BlockType type, int face) {
    if (face < 0 || face >= 6) return 0;
    const BlockInfo* info = block_get_info(type);
    return info->texture_faces[face];
}
