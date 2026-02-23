#ifndef PHYSICS_H
#define PHYSICS_H

#include "math/vec3.h"
#include <stdbool.h>

typedef struct {
    vec3 min;
    vec3 max;
} AABB;

typedef struct {
    vec3 start;
    vec3 direction;
    float length;
} Ray;

// 碰撞检测
bool aabb_intersect(AABB a, AABB b);
bool ray_aabb_intersect(Ray ray, AABB box, float* t);
bool ray_block_intersect(Ray ray, float max_distance, 
                         int* hit_x, int* hit_y, int* hit_z,
                         int* face_x, int* face_y, int* face_z);

// 物理更新
void physics_apply_gravity(vec3* velocity, float gravity, float delta_time);
void physics_apply_friction(vec3* velocity, float friction, float delta_time);
bool physics_check_collision(AABB box);

#endif
