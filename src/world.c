#include "world.h"
#include "chunk.h"
#include "noise.h"
#include "math/vec3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 柏林噪声函数（简单实现）
static float perlin_noise_2d(float x, float z, int seed) {
    // 使用种子初始化
    srand(seed + (int)(x * 1000) + (int)(z * 1000));
    return (float)rand() / RAND_MAX;
}

// 多层噪声叠加
static float fractal_noise(float x, float z, int octaves, float persistence, int seed) {
    float total = 0;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float max_value = 0;
    
    for (int i = 0; i < octaves; i++) {
        total += perlin_noise_2d(x * frequency, z * frequency, seed + i) * amplitude;
        max_value += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / max_value;
}

// 创建世界
World world_create(int seed) {
    World world = {0};
    world.seed = seed;
    world.chunk_count = 0;
    
    // 初始化区块指针数组
    for (int z = 0; z < WORLD_SIZE; z++) {
        for (int x = 0; x < WORLD_SIZE; x++) {
            world.chunks[x][z] = NULL;
        }
    }
    
    // 初始化噪声函数
    world.noise = fractal_noise;
    
    printf("World created with seed: %d\\n", seed);
    return world;
}

// 销毁世界
void world_destroy(World* world) {
    printf("Destroying world...\\n");
    
    for (int z = 0; z < WORLD_SIZE; z++) {
        for (int x = 0; x < WORLD_SIZE; x++) {
            if (world->chunks[x][z]) {
                chunk_destroy(world->chunks[x][z]);
                free(world->chunks[x][z]);
                world->chunks[x][z] = NULL;
            }
        }
    }
    
    world->chunk_count = 0;
    printf("World destroyed\\n");
}

// 生成世界
void world_generate(World* world) {
    printf("Generating world...\\n");
    
    int center = WORLD_SIZE / 2;
    
    for (int cz = 0; cz < WORLD_SIZE; cz++) {
        for (int cx = 0; cx < WORLD_SIZE; cx++) {
            // 创建区块
            Chunk* chunk = malloc(sizeof(Chunk));
            *chunk = chunk_create(cx - center, cz - center);
            
            // 生成区块地形
            for (int z = 0; z < CHUNK_SIZE; z++) {
                for (int x = 0; x < CHUNK_SIZE; x++) {
                    // 计算世界坐标
                    float world_x = (cx - center) * CHUNK_SIZE + x;
                    float world_z = (cz - center) * CHUNK_SIZE + z;
                    
                    // 使用多层噪声生成地形高度
                    float height = 64.0f;  // 基准高度
                    
                    // 地形噪声
                    height += world->noise(world_x * 0.01f, world_z * 0.01f, 4, 0.5f, world->seed) * 32.0f;
                    height += world->noise(world_x * 0.05f, world_z * 0.05f, 2, 0.2f, world->seed) * 8.0f;
                    
                    // 山地噪声
                    float mountain = world->noise(world_x * 0.005f, world_z * 0.005f, 1, 0.5f, world->seed + 1000);
                    if (mountain > 0.6f) {
                        height += (mountain - 0.6f) * 50.0f;
                    }
                    
                    int terrain_height = (int)height;
                    if (terrain_height < 1) terrain_height = 1;
                    if (terrain_height >= CHUNK_HEIGHT) terrain_height = CHUNK_HEIGHT - 1;
                    
                    // 洞穴噪声
                    float cave_noise = world->noise(world_x * 0.1f, world_z * 0.1f, 2, 0.5f, world->seed + 2000);
                    float cave_threshold = 0.3f;
                    
                    // 填充方块
                    for (int y = 0; y < CHUNK_HEIGHT; y++) {
                        BlockType block_type = BLOCK_AIR;
                        
                        // 基岩层
                        if (y == 0) {
                            block_type = BLOCK_BEDROCK;
                        }
                        // 洞穴
                        else if (y < terrain_height - 10 && cave_noise > cave_threshold) {
                            block_type = BLOCK_AIR;
                        }
                        // 石头层
                        else if (y < terrain_height - 4) {
                            block_type = BLOCK_STONE;
                            
                            // 矿石生成
                            if (y < 16) {
                                float ore_noise = world->noise(world_x * 0.2f, y * 0.2f, 1, 0.5f, world->seed + 3000);
                                if (ore_noise > 0.8f) block_type = BLOCK_COBBLESTONE;
                            }
                        }
                        // 泥土层
                        else if (y < terrain_height - 1) {
                            block_type = BLOCK_DIRT;
                        }
                        // 草方块
                        else if (y == terrain_height - 1) {
                            block_type = BLOCK_GRASS;
                        }
                        // 水面（y=64）
                        else if (y <= 64 && y >= terrain_height) {
                            block_type = BLOCK_WATER;
                        }
                        
                        // 放置方块
                        chunk_set_block(chunk, x, y, z, block_type);
                    }
                }
            }
            
            // 生成树木
            for (int z = 2; z < CHUNK_SIZE - 2; z++) {
                for (int x = 2; x < CHUNK_SIZE - 2; x++) {
                    float tree_noise = world->noise(
                        ((cx - center) * CHUNK_SIZE + x) * 0.2f,
                        ((cz - center) * CHUNK_SIZE + z) * 0.2f,
                        1, 0.5f, world->seed + 4000
                    );
                    
                    if (tree_noise > 0.9f) {
                        int world_x = (cx - center) * CHUNK_SIZE + x;
                        int world_z = (cz - center) * CHUNK_SIZE + z;
                        int ground_height = world_get_highest_block_at(world, world_x, world_z);
                        
                        // 确保在地面上
                        if (ground_height >= 60 && ground_height < CHUNK_HEIGHT - 10) {
                            // 树根
                            chunk_set_block(chunk, x, ground_height, z, BLOCK_WOOD);
                            
                            // 树干（4格高）
                            for (int i = 1; i <= 4; i++) {
                                if (ground_height + i < CHUNK_HEIGHT) {
                                    chunk_set_block(chunk, x, ground_height + i, z, BLOCK_WOOD);
                                }
                            }
                            
                            // 树叶
                            for (int dz = -2; dz <= 2; dz++) {
                                for (int dx = -2; dx <= 2; dx++) {
                                    for (int dy = 3; dy <= 5; dy++) {
                                        int leaf_x = x + dx;
                                        int leaf_z = z + dz;
                                        int leaf_y = ground_height + dy;
                                        
                                        if (leaf_x >= 0 && leaf_x < CHUNK_SIZE &&
                                            leaf_z >= 0 && leaf_z < CHUNK_SIZE &&
                                            leaf_y >= 0 && leaf_y < CHUNK_HEIGHT) {
                                            
                                            // 树叶球体
                                            float dist = sqrtf(dx*dx + dz*dz + (dy-4)*(dy-4));
                                            if (dist <= 2.5f) {
                                                chunk_set_block(chunk, leaf_x, leaf_y, leaf_z, BLOCK_LEAVES);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // 构建区块网格
            chunk_rebuild(chunk);
            
            // 保存到世界
            world->chunks[cx][cz] = chunk;
            world->chunk_count++;
            
            if (world->chunk_count % 4 == 0) {
                printf("Generated %d/%d chunks...\\n", 
                       world->chunk_count, WORLD_SIZE * WORLD_SIZE);
            }
        }
    }
    
    printf("World generation completed: %d chunks\\n", world->chunk_count);
}

// 获取出生点
vec3 world_get_spawn(World* world) {
    int center_x = WORLD_SIZE * CHUNK_SIZE / 2;
    int center_z = WORLD_SIZE * CHUNK_SIZE / 2;
    
    // 找到中心点最高方块
    int highest = 0;
    for (int y = CHUNK_HEIGHT - 1; y >= 0; y--) {
        BlockType block = world_get_block(world, center_x, y, center_z);
        if (block != BLOCK_AIR && block != BLOCK_WATER) {
            highest = y;
            break;
        }
    }
    
    // 返回出生点（在最高方块上方）
    return (vec3){
        center_x + 0.5f,
        highest + 2.0f,
        center_z + 0.5f
    };
}

// 获取指定位置最高方块
int world_get_highest_block_at(World* world, int x, int z) {
    for (int y = CHUNK_HEIGHT - 1; y >= 0; y--) {
        BlockType block = world_get_block(world, x, y, z);
        if (block != BLOCK_AIR && block != BLOCK_WATER) {
            return y;
        }
    }
    return 0;
}

// 获取方块
BlockType world_get_block(const World* world, int x, int y, int z) {
    if (y < 0 || y >= CHUNK_HEIGHT) {
        return BLOCK_AIR;
    }
    
    // 计算区块坐标
    int chunk_x = (x >> 4) + (WORLD_SIZE / 2);  // >> 4 相当于 / 16
    int chunk_z = (z >> 4) + (WORLD_SIZE / 2);
    
    // 计算区块内坐标
    int local_x = x & 0xF;  // & 0xF 相当于 % 16
    int local_z = z & 0xF;
    
    // 检查边界
    if (chunk_x < 0 || chunk_x >= WORLD_SIZE || 
        chunk_z < 0 || chunk_z >= WORLD_SIZE) {
        return BLOCK_AIR;
    }
    
    Chunk* chunk = world->chunks[chunk_x][chunk_z];
    if (!chunk) {
        return BLOCK_AIR;
    }
    
    return chunk_get_block(chunk, local_x, y, local_z);
}

// 设置方块
void world_set_block(World* world, int x, int y, int z, BlockType type) {
    if (y < 0 || y >= CHUNK_HEIGHT) {
        return;
    }
    
    // 计算区块坐标
    int chunk_x = (x >> 4) + (WORLD_SIZE / 2);
    int chunk_z = (z >> 4) + (WORLD_SIZE / 2);
    
    // 计算区块内坐标
    int local_x = x & 0xF;
    int local_z = z & 0xF;
    
    // 检查边界
    if (chunk_x < 0 || chunk_x >= WORLD_SIZE || 
        chunk_z < 0 || chunk_z >= WORLD_SIZE) {
        return;
    }
    
    Chunk* chunk = world->chunks[chunk_x][chunk_z];
    if (!chunk) {
        return;
    }
    
    // 设置方块
    chunk_set_block(chunk, local_x, y, local_z, type);
    
    // 标记需要重建网格
    chunk->needs_rebuild = true;
    
    // 如果改变了边界方块，需要更新相邻区块
    if (local_x == 0) {
        Chunk* neighbor = world_get_chunk_safe(world, chunk_x - 1, chunk_z);
        if (neighbor) neighbor->needs_rebuild = true;
    } else if (local_x == CHUNK_SIZE - 1) {
        Chunk* neighbor = world_get_chunk_safe(world, chunk_x + 1, chunk_z);
        if (neighbor) neighbor->needs_rebuild = true;
    }
    
    if (local_z == 0) {
        Chunk* neighbor = world_get_chunk_safe(world, chunk_x, chunk_z - 1);
        if (neighbor) neighbor->needs_rebuild = true;
    } else if (local_z == CHUNK_SIZE - 1) {
        Chunk* neighbor = world_get_chunk_safe(world, chunk_x, chunk_z + 1);
        if (neighbor) neighbor->needs_rebuild = true;
    }
}

// 安全获取区块
static Chunk* world_get_chunk_safe(World* world, int chunk_x, int chunk_z) {
    if (chunk_x < 0 || chunk_x >= WORLD_SIZE || 
        chunk_z < 0 || chunk_z >= WORLD_SIZE) {
        return NULL;
    }
    return world->chunks[chunk_x][chunk_z];
}

// 更新世界（重新构建需要更新的区块）
void world_update(World* world, vec3 player_pos) {
    int updated = 0;
    
    for (int z = 0; z < WORLD_SIZE; z++) {
        for (int x = 0; x < WORLD_SIZE; x++) {
            Chunk* chunk = world->chunks[x][z];
            if (chunk && chunk->needs_rebuild) {
                chunk_rebuild(chunk);
                updated++;
            }
        }
    }
    
    if (updated > 0) {
        printf("Updated %d chunks\\n", updated);
    }
}

// 渲染世界
void world_render(const World* world) {
    int rendered = 0;
    
    for (int z = 0; z < WORLD_SIZE; z++) {
        for (int x = 0; x < WORLD_SIZE; x++) {
            Chunk* chunk = world->chunks[x][z];
            if (chunk && !chunk->is_empty) {
                chunk_render(chunk);
                rendered++;
            }
        }
    }
    
    static int last_rendered = 0;
    if (rendered != last_rendered) {
        last_rendered = rendered;
        printf("Rendered %d chunks\\n", rendered);
    }
}

// 获取世界信息
int world_get_seed(const World* world) {
    return world->seed;
}

int world_get_chunk_count(const World* world) {
    return world->chunk_count;
}
