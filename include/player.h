#ifndef PLAYER_H
#define PLAYER_H

#include "math/vec3.h"
#include "camera.h"
#include <stdbool.h>

typedef struct {
    vec3 position;
    vec3 velocity;
    vec3 size;  // 碰撞箱大小
    
    Camera camera;
    
    float speed;
    float sprint_speed;
    float jump_force;
    float gravity;
    
    bool on_ground;
    bool flying;
    bool sprinting;
    bool sneaking;
    
    int selected_slot;  // 物品栏选择
} Player;

Player player_create(vec3 position);
void player_update(Player* player, float delta_time);
void player_handle_input(Player* player);
void player_move(Player* player, vec3 movement);
void player_jump(Player* player);
void player_toggle_fly(Player* player);
vec3 player_get_look_direction(const Player* player);

#endif
