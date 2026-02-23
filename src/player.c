#include "player.h"
#include "input.h"
#include "physics.h"
#include "world.h"
#include "math/vec3.h"
#include "math/mat4.h"
#include <stdio.h>
#include <math.h>

// 获取玩家的AABB碰撞箱
static AABB player_get_aabb(const Player* player) {
    vec3 half_size = vec3_scale(player->size, 0.5f);
    return (AABB){
        .min = vec3_sub(player->position, half_size),
        .max = vec3_add(player->position, half_size)
    };
}

// 创建玩家
Player player_create(vec3 position) {
    Player player = {0};
    
    player.position = position;
    player.velocity = VEC3_ZERO;
    player.size = (vec3){0.6f, 1.8f, 0.6f};  // Minecraft玩家尺寸
    
    // 摄像机设置
    player.camera.position = player.position;
    player.camera.front = (vec3){0, 0, -1};
    player.camera.up = VEC3_UP;
    player.camera.right = VEC3_RIGHT;
    player.camera.world_up = VEC3_UP;
    player.camera.yaw = -90.0f;
    player.camera.pitch = 0.0f;
    player.camera.fov = 70.0f;
    player.camera.aspect = 16.0f / 9.0f;
    player.camera.near = 0.1f;
    player.camera.far = 1000.0f;
    player.camera.needs_update = 1;
    
    // 移动参数
    player.speed = 5.0f;
    player.sprint_speed = 8.0f;
    player.jump_force = 8.0f;
    player.gravity = 20.0f;
    
    // 状态
    player.on_ground = false;
    player.flying = false;
    player.sprinting = false;
    player.sneaking = false;
    player.in_water = false;
    
    // 交互
    player.selected_slot = 0;
    player.reach_distance = 5.0f;
    
    printf("Player created at (%.1f, %.1f, %.1f)\\n", 
           position.x, position.y, position.z);
    
    return player;
}

// 处理键盘输入
static void player_handle_keyboard(Player* player, float delta_time) {
    float current_speed = player->sprinting ? player->sprint_speed : player->speed;
    
    // 在水中减速
    if (player->in_water) {
        current_speed *= 0.5f;
    }
    
    // 潜行
    if (input_key_down(KEY_LEFT_SHIFT)) {
        player->sneaking = true;
        current_speed = 1.5f;
        player->size.y = 1.2f;  // 变矮
    } else {
        player->sneaking = false;
        player->size.y = 1.8f;  // 恢复正常高度
    }
    
    // 冲刺
    player->sprinting = input_key_down(KEY_LEFT_CONTROL);
    
    // 飞行切换
    static bool f_pressed = false;
    if (input_key_down(KEY_F) && !f_pressed) {
        player->flying = !player->flying;
        printf("Flying mode: %s\\n", player->flying ? "ON" : "OFF");
        f_pressed = true;
    } else if (!input_key_down(KEY_F)) {
        f_pressed = false;
    }
    
    // 移动方向
    vec3 move_dir = VEC3_ZERO;
    vec3 front = player->camera.front;
    vec3 right = player->camera.right;
    
    // 向前/后
    if (input_key_down(KEY_W)) {
        move_dir = vec3_add(move_dir, front);
    }
    if (input_key_down(KEY_S)) {
        move_dir = vec3_sub(move_dir, front);
    }
    
    // 向左/右
    if (input_key_down(KEY_A)) {
        move_dir = vec3_sub(move_dir, right);
    }
    if (input_key_down(KEY_D)) {
        move_dir = vec3_add(move_dir, right);
    }
    
    // 向上/下（飞行模式）
    if (player->flying) {
        if (input_key_down(KEY_SPACE)) {
            move_dir.y += 1.0f;
        }
        if (input_key_down(KEY_LEFT_SHIFT)) {
            move_dir.y -= 1.0f;
        }
    }
    
    // 归一化移动方向
    if (vec3_length_squared(move_dir) > 0) {
        move_dir = vec3_normalize(move_dir);
        
        // 应用速度
        player->velocity.x = move_dir.x * current_speed;
        player->velocity.z = move_dir.z * current_speed;
        
        if (player->flying) {
            player->velocity.y = move_dir.y * current_speed;
        }
    } else {
        // 没有输入时减速
        if (!player->flying) {
            player->velocity.x *= 0.8f;
            player->velocity.z *= 0.8f;
        }
    }
    
    // 跳跃
    static bool space_pressed = false;
    if (input_key_down(KEY_SPACE) && !space_pressed) {
        if (player->on_ground && !player->flying) {
            player->velocity.y = player->jump_force;
            player->on_ground = false;
        }
        space_pressed = true;
    } else if (!input_key_down(KEY_SPACE)) {
        space_pressed = false;
    }
}

// 处理鼠标输入
static void player_handle_mouse(Player* player) {
    static vec2 last_mouse_pos = {0, 0};
    vec2 mouse_pos = input_get_mouse_position();
    
    // 第一次调用
    if (last_mouse_pos.x == 0 && last_mouse_pos.y == 0) {
        last_mouse_pos = mouse_pos;
        return;
    }
    
    float x_offset = mouse_pos.x - last_mouse_pos.x;
    float y_offset = last_mouse_pos.y - mouse_pos.y;  // 反转Y轴
    
    last_mouse_pos = mouse_pos;
    
    float sensitivity = 0.1f;
    x_offset *= sensitivity;
    y_offset *= sensitivity;
    
    // 更新旋转
    player->camera.yaw += x_offset;
    player->camera.pitch += y_offset;
    
    // 限制俯仰角
    if (player->camera.pitch > 89.0f) player->camera.pitch = 89.0f;
    if (player->camera.pitch < -89.0f) player->camera.pitch = -89.0f;
    
    player->camera.needs_update = 1;
}

// 处理鼠标点击（放置/破坏方块）
static void player_handle_clicks(Player* player) {
    static bool left_pressed = false;
    static bool right_pressed = false;
    
    // 左键破坏方块
    if (input_mouse_down(MOUSE_BUTTON_LEFT) && !left_pressed) {
        Ray ray = physics_get_look_ray(
            player->camera.position,
            player->camera.front,
            player->reach_distance
        );
        
        int hit_x, hit_y, hit_z;
        int face_x, face_y, face_z;
        
        if (ray_block_intersect(ray, player->reach_distance,
                               &hit_x, &hit_y, &hit_z,
                               &face_x, &face_y, &face_z)) {
            // 破坏方块
            world_set_block(hit_x, hit_y, hit_z, BLOCK_AIR);
            printf("Block destroyed at (%d, %d, %d)\\n", hit_x, hit_y, hit_z);
        }
        
        left_pressed = true;
    } else if (!input_mouse_down(MOUSE_BUTTON_LEFT)) {
        left_pressed = false;
    }
    
    // 右键放置方块
    if (input_mouse_down(MOUSE_BUTTON_RIGHT) && !right_pressed) {
        Ray ray = physics_get_look_ray(
            player->camera.position,
            player->camera.front,
            player->reach_distance
        );
        
        int hit_x, hit_y, hit_z;
        int face_x, face_y, face_z;
        
        if (ray_block_intersect(ray, player->reach_distance,
                               &hit_x, &hit_y, &hit_z,
                               &face_x, &face_y, &face_z)) {
            // 在点击的面旁边放置方块
            int place_x = hit_x + face_x;
            int place_y = hit_y + face_y;
            int place_z = hit_z + face_z;
            
            // 获取当前选中的方块类型
            BlockType block_to_place = BLOCK_STONE;  // 默认石头
            
            // 检查是否可以放置
            AABB block_aabb = aabb_from_pos_size(
                (vec3){place_x, place_y, place_z},
                (vec3){1.0f, 1.0f, 1.0f}
            );
            
            if (!physics_check_collision(block_aabb)) {
                world_set_block(place_x, place_y, place_z, block_to_place);
                printf("Block placed at (%d, %d, %d)\\n", place_x, place_y, place_z);
            }
        }
        
        right_pressed = true;
    } else if (!input_mouse_down(MOUSE_BUTTON_RIGHT)) {
        right_pressed = false;
    }
}

// 处理滚轮选择物品栏
static void player_handle_scroll(Player* player) {
    vec2 scroll = input_get_mouse_scroll();
    
    if (scroll.y > 0) {
        player->selected_slot--;
        if (player->selected_slot < 0) player->selected_slot = 8;
    } else if (scroll.y < 0) {
        player->selected_slot++;
        if (player->selected_slot > 8) player->selected_slot = 0;
    }
}

// 更新玩家
void player_update(Player* player, float delta_time) {
    // 处理输入
    player_handle_keyboard(player, delta_time);
    player_handle_mouse(player);
    player_handle_clicks(player);
    player_handle_scroll(player);
    
    // 更新摄像机位置
    vec3 eye_position = player->position;
    eye_position.y += 1.6f;  // 眼睛高度
    
    if (player->sneaking) {
        eye_position.y -= 0.3f;  // 潜行时眼睛降低
    }
    
    camera_set_position(&player->camera, eye_position);
    camera_update(&player->camera);
    
    // 物理更新
    AABB player_box = player_get_aabb(player);
    
    // 检测是否在地面
    player->on_ground = physics_is_on_ground(player_box);
    
    // 检测是否在水中
    player->in_water = physics_is_in_water(player_box);
    
    // 应用重力（如果不飞行且不在地面）
    if (!player->flying && !player->on_ground) {
        physics_apply_gravity(&player->velocity, player->gravity, delta_time);
    } else if (player->on_ground) {
        // 在地面时Y速度归零
        player->velocity.y = 0.0f;
    }
    
    // 应用摩擦力
    if (player->on_ground) {
        physics_apply_friction(&player->velocity, 5.0f, delta_time);
    }
    
    // 解决碰撞
    vec3 move_amount = vec3_scale(player->velocity, delta_time);
    vec3 resolved_move = physics_resolve_collision(player_box, move_amount);
    
    // 更新位置
    player->position = vec3_add(player->position, resolved_move);
    
    // 如果Y轴移动被阻挡，重置Y速度
    if (fabsf(resolved_move.y - move_amount.y) > 0.001f) {
        if (player->velocity.y < 0) {
            player->on_ground = true;
        }
        player->velocity.y = 0.0f;
    }
    
    // 限制玩家高度（不能低于0）
    if (player->position.y < 0) {
        player->position.y = 0;
        player->velocity.y = 0;
        player->on_ground = true;
    }
}

// 获取玩家视线方向
vec3 player_get_look_direction(const Player* player) {
    return player->camera.front;
}

// 获取玩家视线射线
Ray player_get_look_ray(const Player* player) {
    return physics_get_look_ray(
        player->camera.position,
        player->camera.front,
        player->reach_distance
    );
}

// 切换飞行模式
void player_toggle_fly(Player* player) {
    player->flying = !player->flying;
    printf("Flying mode: %s\\n", player->flying ? "ON" : "OFF");
}

// 跳跃
void player_jump(Player* player) {
    if (player->on_ground && !player->flying) {
        player->velocity.y = player->jump_force;
        player->on_ground = false;
    }
}

// 设置玩家位置
void player_set_position(Player* player, vec3 position) {
    player->position = position;
    
    // 更新摄像机位置
    vec3 eye_position = position;
    eye_position.y += 1.6f;
    camera_set_position(&player->camera, eye_position);
    camera_update(&player->camera);
}

// 获取玩家位置
vec3 player_get_position(const Player* player) {
    return player->position;
}

// 获取玩家摄像机
Camera* player_get_camera(Player* player) {
    return &player->camera;
}
