#include "math/vec3.h"
#include "math/mat4.h"
#include "window.h"
#include "math/vec3.h"
#include "texture.h"
#include "camera.h"
#include "block.h"
#include "chunk.h"
#include "world.h"
#include "player.h"
#include "renderer.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 全局变量
static Window* g_window = NULL;
static Shader g_block_shader = {0};
static Camera g_camera = {0};
static World g_world = {0};
static Player g_player = {0};
static Renderer g_renderer = {0};
static TextureArray g_block_textures = {0};
static double g_last_time = 0.0;
static int g_fps = 0;
static int g_frame_count = 0;
static double g_fps_timer = 0.0;

// 初始化函数
static bool init() {
    printf("=== Minecraft Clone Initialization ===\\n");
    
    // 1. 初始化随机种子
    srand(time(NULL));
    
    // 2. 创建窗口
    g_window = window_create(1280, 720, "Minecraft Clone");
    if (!g_window) {
        fprintf(stderr, "Failed to create window\\n");
        return false;
    }
    
    // 3. 初始化输入系统
    input_init(g_window);
    
    // 4. 初始化渲染器
    g_renderer = renderer_create(1280, 720);
    
    // 5. 加载方块着色器
    g_block_shader = shader_create(
        "assets/shaders/block.vert",
        "assets/shaders/block.frag"
    );
    
    if (g_block_shader.id == 0) {
        fprintf(stderr, "Failed to load shaders\\n");
        return false;
    }
    
    // 6. 加载方块纹理
    g_block_textures = texture_array_create(
        "assets/textures/blocks.png",
        16,  // 瓦片大小
        256  // 瓦片数量
    );
    
    if (g_block_textures.id == 0) {
        fprintf(stderr, "Failed to load block textures\\n");
        return false;
    }
    
    // 7. 创建摄像机
    g_camera = camera_create(
        70.0f,    // 视野
        1280.0f / 720.0f,  // 宽高比
        0.1f,     // 近平面
        1000.0f   // 远平面
    );
    
    // 8. 创建世界
    g_world = world_create(rand());
    world_generate(&g_world);
    
    // 9. 创建玩家
    vec3 spawn_point = world_get_spawn(&g_world);
    g_player = player_create(spawn_point);
    
    printf("Initialization completed successfully!\\n");
    printf("World seed: %d\\n", g_world.seed);
    printf("Spawn point: (%.1f, %.1f, %.1f)\\n", 
           spawn_point.x, spawn_point.y, spawn_point.z);
    
    return true;
}

// 更新函数
static void update(double delta_time) {
    // 1. 更新输入
    input_update();
    
    // 2. 处理退出
    if (input_key_down(KEY_ESCAPE)) {
        window_set_should_close(g_window, true);
    }
    
    // 3. 更新玩家
    player_update(&g_player, (float)delta_time);
    
    // 4. 更新摄像机位置
    camera_set_position(&g_camera, g_player.position);
    camera_set_rotation(&g_camera, g_player.yaw, g_player.pitch);
    camera_update(&g_camera);
    
    // 5. 更新世界
    world_update(&g_world, g_player.position);
    
    // 6. 更新FPS计数
    g_frame_count++;
    g_fps_timer += delta_time;
    if (g_fps_timer >= 1.0) {
        g_fps = g_frame_count;
        g_frame_count = 0;
        g_fps_timer = 0.0;
        
        // 显示FPS和位置
        printf("FPS: %d, Pos: (%.1f, %.1f, %.1f)\\n", 
               g_fps, g_player.position.x, g_player.position.y, g_player.position.z);
    }
}

// 渲染函数
static void render() {
    // 1. 开始新的一帧
    renderer_begin_frame(&g_renderer);
    
    // 2. 清屏
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);  // 天空蓝色
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 3. 使用方块着色器
    shader_use(g_block_shader);
    
    // 4. 设置着色器uniform
    shader_set_mat4(g_block_shader, "projection", g_camera.projection.a);
    shader_set_mat4(g_block_shader, "view", g_camera.view.a);
    shader_set_vec3(g_block_shader, "view_pos", 
                    g_camera.position.x, g_camera.position.y, g_camera.position.z);
    
    // 5. 绑定方块纹理
    texture_array_bind(g_block_textures, 0);
    shader_set_int(g_block_shader, "texture_array", 0);
    
    // 6. 渲染世界
    world_render(&g_world);
    
    // 7. 渲染准星
    renderer_draw_crosshair(&g_renderer);
    
    // 8. 渲染天空
    renderer_draw_sky(&g_renderer, &g_camera);
    
    // 9. 结束帧
    renderer_end_frame(&g_renderer);
}

// 清理函数
static void cleanup() {
    printf("\\n=== Cleaning up ===\\n");
    
    // 销毁世界
    world_destroy(&g_world);
    
    // 销毁渲染器
    renderer_destroy(&g_renderer);
    
    // 销毁着色器
    shader_destroy(g_block_shader);
    
    // 销毁纹理
    glDeleteTextures(1, &g_block_textures.id);
    
    // 销毁窗口
    window_destroy(g_window);
    
    printf("Cleanup completed. Goodbye!\\n");
}

// 主函数
int main(int argc, char** argv) {
    printf("=== Starting Minecraft Clone ===\\n");
    
    // 1. 初始化
    if (!init()) {
        fprintf(stderr, "Initialization failed\\n");
        return 1;
    }
    
    // 2. 主循环
    g_last_time = glfwGetTime();
    
    while (!window_should_close(g_window)) {
        // 计算delta time
        double current_time = glfwGetTime();
        double delta_time = current_time - g_last_time;
        g_last_time = current_time;
        
        // 限制最大delta time
        if (delta_time > 0.1) {
            delta_time = 0.1;
        }
        
        // 处理事件
        window_poll_events();
        
        // 更新游戏逻辑
        update(delta_time);
        
        // 渲染
        render();
        
        // 交换缓冲区
        window_swap_buffers(g_window);
    }
    
    // 3. 清理
    cleanup();
    
    return 0;
}
