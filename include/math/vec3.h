#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <stdbool.h>

// 三维向量
typedef struct { 
    float x, y, z; 
} vec3;

// 常量定义
#define VEC3_ZERO ((vec3){0, 0, 0})
#define VEC3_ONE ((vec3){1, 1, 1})
#define VEC3_UP ((vec3){0, 1, 0})
#define VEC3_DOWN ((vec3){0, -1, 0})
#define VEC3_LEFT ((vec3){-1, 0, 0})
#define VEC3_RIGHT ((vec3){1, 0, 0})
#define VEC3_FORWARD ((vec3){0, 0, 1})
#define VEC3_BACK ((vec3){0, 0, -1})

// 基本运算
static inline vec3 vec3_add(vec3 a, vec3 b) { 
    return (vec3){a.x + b.x, a.y + b.y, a.z + b.z}; 
}

static inline vec3 vec3_sub(vec3 a, vec3 b) { 
    return (vec3){a.x - b.x, a.y - b.y, a.z - b.z}; 
}

static inline vec3 vec3_mul(vec3 a, vec3 b) { 
    return (vec3){a.x * b.x, a.y * b.y, a.z * b.z}; 
}

static inline vec3 vec3_div(vec3 a, vec3 b) { 
    return (vec3){a.x / b.x, a.y / b.y, a.z / b.z}; 
}

static inline vec3 vec3_scale(vec3 v, float s) { 
    return (vec3){v.x * s, v.y * s, v.z * s}; 
}

// 点积
static inline float vec3_dot(vec3 a, vec3 b) { 
    return a.x * b.x + a.y * b.y + a.z * b.z; 
}

// 叉积
static inline vec3 vec3_cross(vec3 a, vec3 b) {
    return (vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// 长度
static inline float vec3_length(vec3 v) { 
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); 
}

static inline float vec3_length_squared(vec3 v) { 
    return v.x * v.x + v.y * v.y + v.z * v.z; 
}

// 归一化
static inline vec3 vec3_normalize(vec3 v) {
    float len = vec3_length(v);
    if (len > 0.0f) {
        return vec3_scale(v, 1.0f / len);
    }
    return VEC3_ZERO;
}

// 距离
static inline float vec3_distance(vec3 a, vec3 b) {
    return vec3_length(vec3_sub(a, b));
}

static inline float vec3_distance_squared(vec3 a, vec3 b) {
    vec3 d = vec3_sub(a, b);
    return vec3_dot(d, d);
}

// 线性插值
static inline vec3 vec3_lerp(vec3 a, vec3 b, float t) {
    return (vec3){
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    };
}

// 反射
static inline vec3 vec3_reflect(vec3 v, vec3 n) {
    float dot = vec3_dot(v, n);
    return vec3_sub(v, vec3_scale(n, 2.0f * dot));
}

// 比较
static inline bool vec3_equals(vec3 a, vec3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

static inline bool vec3_approx_equals(vec3 a, vec3 b, float epsilon) {
    return fabsf(a.x - b.x) < epsilon &&
           fabsf(a.y - b.y) < epsilon &&
           fabsf(a.z - b.z) < epsilon;
}

// 最小值/最大值
static inline vec3 vec3_min(vec3 a, vec3 b) {
    return (vec3){
        fminf(a.x, b.x),
        fminf(a.y, b.y),
        fminf(a.z, b.z)
    };
}

static inline vec3 vec3_max(vec3 a, vec3 b) {
    return (vec3){
        fmaxf(a.x, b.x),
        fmaxf(a.y, b.y),
        fmaxf(a.z, b.z)
    };
}

// 数学运算
static inline vec3 vec3_floor(vec3 v) {
    return (vec3){
        floorf(v.x),
        floorf(v.y),
        floorf(v.z)
    };
}

static inline vec3 vec3_ceil(vec3 v) {
    return (vec3){
        ceilf(v.x),
        ceilf(v.y),
        ceilf(v.z)
    };
}

static inline vec3 vec3_round(vec3 v) {
    return (vec3){
        roundf(v.x),
        roundf(v.y),
        roundf(v.z)
    };
}

static inline vec3 vec3_abs(vec3 v) {
    return (vec3){
        fabsf(v.x),
        fabsf(v.y),
        fabsf(v.z)
    };
}

#endif
