#include "physics.h"
#include "world.h"
#include "block.h"
#include "math/vec3.h"
#include <math.h>
#include <stdbool.h>

// 检查两个AABB是否相交
bool aabb_intersect(AABB a, AABB b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

// 计算AABB的中心点
static vec3 aabb_center(AABB box) {
    return (vec3){
        (box.min.x + box.max.x) * 0.5f,
        (box.min.y + box.max.y) * 0.5f,
        (box.min.z + box.max.z) * 0.5f
    };
}

// 计算AABB的大小
static vec3 aabb_size(AABB box) {
    return (vec3){
        box.max.x - box.min.x,
        box.max.y - box.min.y,
        box.max.z - box.min.z
    };
}

// 从位置和大小创建AABB
AABB aabb_from_pos_size(vec3 position, vec3 size) {
    vec3 half_size = vec3_scale(size, 0.5f);
    return (AABB){
        .min = vec3_sub(position, half_size),
        .max = vec3_add(position, half_size)
    };
}

// 射线与AABB相交检测
bool ray_aabb_intersect(Ray ray, AABB box, float* t_near, float* t_far) {
    vec3 inv_dir = (vec3){
        1.0f / ray.direction.x,
        1.0f / ray.direction.y,
        1.0f / ray.direction.z
    };
    
    vec3 t1 = vec3_mul(vec3_sub(box.min, ray.start), inv_dir);
    vec3 t2 = vec3_mul(vec3_sub(box.max, ray.start), inv_dir);
    
    vec3 t_min = vec3_min(t1, t2);
    vec3 t_max = vec3_max(t1, t2);
    
    float t_min_max = fmaxf(fmaxf(t_min.x, t_min.y), t_min.z);
    float t_max_min = fminf(fminf(t_max.x, t_max.y), t_max.z);
    
    if (t_max_min < 0 || t_min_max > t_max_min) {
        return false;
    }
    
    if (t_near) *t_near = t_min_max;
    if (t_far) *t_far = t_max_min;
    
    return true;
}

// 射线与方块相交检测（返回命中的方块和面）
bool ray_block_intersect(Ray ray, float max_distance,
                         int* hit_x, int* hit_y, int* hit_z,
                         int* face_x, int* face_y, int* face_z) {
    // 从起点向各方向取整
    int x = (int)floorf(ray.start.x);
    int y = (int)floorf(ray.start.y);
    int z = (int)floorf(ray.start.z);
    
    int step_x = (ray.direction.x > 0) ? 1 : -1;
    int step_y = (ray.direction.y > 0) ? 1 : -1;
    int step_z = (ray.direction.z > 0) ? 1 : -1;
    
    // 到下一个整数边界的距离
    float t_max_x = (ray.direction.x != 0) ? 
        ((x + (step_x > 0 ? 1 : 0) - ray.start.x) / ray.direction.x) : INFINITY;
    float t_max_y = (ray.direction.y != 0) ? 
        ((y + (step_y > 0 ? 1 : 0) - ray.start.y) / ray.direction.y) : INFINITY;
    float t_max_z = (ray.direction.z != 0) ? 
        ((z + (step_z > 0 ? 1 : 0) - ray.start.z) / ray.direction.z) : INFINITY;
    
    // 步进距离
    float t_delta_x = (ray.direction.x != 0) ? fabsf(1.0f / ray.direction.x) : INFINITY;
    float t_delta_y = (ray.direction.y != 0) ? fabsf(1.0f / ray.direction.y) : INFINITY;
    float t_delta_z = (ray.direction.z != 0) ? fabsf(1.0f / ray.direction.z) : INFINITY;
    
    float t = 0.0f;
    int face = 0;  // 0=x, 1=y, 2=z
    
    while (t < max_distance) {
        // 检查当前方块
        BlockType block = world_get_block(x, y, z);
        if (block != BLOCK_AIR && !block_is_transparent(block)) {
            if (hit_x) *hit_x = x;
            if (hit_y) *hit_y = y;
            if (hit_z) *hit_z = z;
            
            // 计算命中的面
            if (face_x) *face_x = (face == 0) ? -step_x : 0;
            if (face_y) *face_y = (face == 1) ? -step_y : 0;
            if (face_z) *face_z = (face == 2) ? -step_z : 0;
            
            return true;
        }
        
        // 步进到下一个方块
        if (t_max_x < t_max_y && t_max_x < t_max_z) {
            x += step_x;
            t = t_max_x;
            t_max_x += t_delta_x;
            face = 0;
        } else if (t_max_y < t_max_z) {
            y += step_y;
            t = t_max_y;
            t_max_y += t_delta_y;
            face = 1;
        } else {
            z += step_z;
            t = t_max_z;
            t_max_z += t_delta_z;
            face = 2;
        }
    }
    
    return false;
}

// 应用重力
void physics_apply_gravity(vec3* velocity, float gravity, float delta_time) {
    velocity->y -= gravity * delta_time;
    
    // 限制最大下落速度
    if (velocity->y < -20.0f) {
        velocity->y = -20.0f;
    }
}

// 应用摩擦力
void physics_apply_friction(vec3* velocity, float friction, float delta_time) {
    float speed = vec3_length(*velocity);
    if (speed > 0) {
        float reduction = friction * delta_time;
        if (reduction > speed) reduction = speed;
        
        vec3 dir = vec3_normalize(*velocity);
        *velocity = vec3_sub(*velocity, vec3_scale(dir, reduction));
    }
}

// 检查碰撞（与世界中的方块）
bool physics_check_collision(AABB box) {
    // 将AABB转换为方块范围
    int min_x = (int)floorf(box.min.x);
    int max_x = (int)ceilf(box.max.x);
    int min_y = (int)floorf(box.min.y);
    int max_y = (int)ceilf(box.max.y);
    int min_z = (int)floorf(box.min.z);
    int max_z = (int)ceilf(box.max.z);
    
    // 检查所有可能碰撞的方块
    for (int y = min_y; y <= max_y; y++) {
        for (int z = min_z; z <= max_z; z++) {
            for (int x = min_x; x <= max_x; x++) {
                BlockType block = world_get_block(x, y, z);
                if (block != BLOCK_AIR && block_is_solid(block)) {
                    // 方块AABB
                    AABB block_box = {
                        .min = {x - 0.5f, y - 0.5f, z - 0.5f},
                        .max = {x + 0.5f, y + 0.5f, z + 0.5f}
                    };
                    
                    if (aabb_intersect(box, block_box)) {
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

// 解决碰撞（将物体推出方块）
vec3 physics_resolve_collision(AABB box, vec3 velocity) {
    vec3 resolved = velocity;
    
    // 分别检查每个轴
    AABB test_box = box;
    
    // X轴
    test_box.min.x += velocity.x;
    test_box.max.x += velocity.x;
    if (physics_check_collision(test_box)) {
        resolved.x = 0.0f;
    }
    
    // Y轴
    test_box = box;
    test_box.min.y += velocity.y;
    test_box.max.y += velocity.y;
    if (physics_check_collision(test_box)) {
        resolved.y = 0.0f;
    }
    
    // Z轴
    test_box = box;
    test_box.min.z += velocity.z;
    test_box.max.z += velocity.z;
    if (physics_check_collision(test_box)) {
        resolved.z = 0.0f;
    }
    
    return resolved;
}

// 检查玩家是否站在地面上
bool physics_is_on_ground(AABB box) {
    // 向下扩展一点点检测
    AABB ground_box = box;
    ground_box.min.y -= 0.1f;
    ground_box.max.y -= 0.1f;
    
    return physics_check_collision(ground_box);
}

// 简单的水物理
bool physics_is_in_water(AABB box) {
    int center_x = (int)floorf(aabb_center(box).x);
    int center_y = (int)floorf(aabb_center(box).y);
    int center_z = (int)floorf(aabb_center(box).z);
    
    BlockType block = world_get_block(center_x, center_y, center_z);
    return block == BLOCK_WATER;
}

// 获取视线方向（从摄像机）
Ray physics_get_look_ray(vec3 position, vec3 direction, float length) {
    return (Ray){
        .start = position,
        .direction = vec3_normalize(direction),
        .length = length
    };
}

// 计算方块被点击的面
int physics_get_hit_face(vec3 hit_pos, vec3 normal) {
    if (normal.x > 0.5f) return 0;  // 东面
    if (normal.x < -0.5f) return 1; // 西面
    if (normal.y > 0.5f) return 2;  // 上面
    if (normal.y < -0.5f) return 3; // 下面
    if (normal.z > 0.5f) return 4;  // 南面
    if (normal.z < -0.5f) return 5; // 北面
    return 0;
}

