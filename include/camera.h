#ifndef CAMERA_H
#define CAMERA_H

#include "math/vec3.h"
#include "math/mat4.h"

typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 world_up;
    
    float yaw;
    float pitch;
    
    float fov;
    float aspect;
    float near;
    float far;
    
    mat4 view;
    mat4 projection;
    int needs_update;
} Camera;

Camera camera_create(float fov, float aspect, float near, float far);
void camera_update(Camera* camera);
void camera_set_position(Camera* camera, vec3 position);
void camera_set_rotation(Camera* camera, float yaw, float pitch);
void camera_move(Camera* camera, vec3 offset);
void camera_rotate(Camera* camera, float yaw_offset, float pitch_offset);
mat4 camera_get_view(Camera* camera);
mat4 camera_get_projection(Camera* camera);

#endif
