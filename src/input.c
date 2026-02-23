#include "input.h"
#include "window.h"
#include "math.h"
#include <string.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

// 输入状态
static struct {
    // 键盘状态
    bool keys[512];
    bool keys_prev[512];
    
    // 鼠标状态
    bool mouse_buttons[8];
    bool mouse_buttons_prev[8];
    vec3 mouse_position;
    vec3 mouse_delta;
    vec3 mouse_scroll;
    
    // 窗口句柄
    GLFWwindow* window;
    
    // 鼠标锁定
    bool mouse_locked;
    vec3 mouse_lock_pos;
} g_input = {0};

// GLFW键盘回调
static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 512) {
        if (action == GLFW_PRESS) {
            g_input.keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            g_input.keys[key] = false;
        }
    }
}

// GLFW鼠标按钮回调
static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button >= 0 && button < 8) {
        if (action == GLFW_PRESS) {
            g_input.mouse_buttons[button] = true;
        } else if (action == GLFW_RELEASE) {
            g_input.mouse_buttons[button] = false;
        }
    }
}

// GLFW鼠标位置回调
static void glfw_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    vec3 new_pos = {(float)xpos, (float)ypos};
    
    if (!g_input.mouse_locked) {
        g_input.mouse_position = new_pos;
        return;
    }
    
    // 计算增量
    g_input.mouse_delta.x = new_pos.x - g_input.mouse_lock_pos.x;
    g_input.mouse_delta.y = new_pos.y - g_input.mouse_lock_pos.y;
    
    // 重置鼠标位置
    glfwSetCursorPos(window, g_input.mouse_lock_pos.x, g_input.mouse_lock_pos.y);
}

// GLFW鼠标滚轮回调
static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    g_input.mouse_scroll.x += (float)xoffset;
    g_input.mouse_scroll.y += (float)yoffset;
}

// 初始化输入系统
void input_init(void* window) {
    memset(&g_input, 0, sizeof(g_input));
    g_input.window = (GLFWwindow*)window;
    
    // 设置GLFW回调
    glfwSetKeyCallback(g_input.window, glfw_key_callback);
    glfwSetMouseButtonCallback(g_input.window, glfw_mouse_button_callback);
    glfwSetCursorPosCallback(g_input.window, glfw_cursor_pos_callback);
    glfwSetScrollCallback(g_input.window, glfw_scroll_callback);
    
    // 隐藏光标
    glfwSetInputMode(g_input.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    g_input.mouse_locked = true;
    
    // 获取初始鼠标位置
    double x, y;
    glfwGetCursorPos(g_input.window, &x, &y);
    g_input.mouse_lock_pos = (vec3){(float)x, (float)y};
    g_input.mouse_position = g_input.mouse_lock_pos;
    
    printf("Input system initialized\\n");
}

// 更新输入状态（每帧调用）
void input_update() {
    // 保存上一帧状态
    memcpy(g_input.keys_prev, g_input.keys, sizeof(g_input.keys));
    memcpy(g_input.mouse_buttons_prev, g_input.mouse_buttons, 
           sizeof(g_input.mouse_buttons));
    
    // 重置增量
    g_input.mouse_delta = (vec3){0, 0};
    g_input.mouse_scroll = (vec3){0, 0};
    
    // 处理事件
    glfwPollEvents();
}

// 检查按键是否按下
bool input_key_down(int key) {
    if (key >= 0 && key < 512) {
        return g_input.keys[key];
    }
    return false;
}

// 检查按键是否刚刚按下（按下瞬间）
bool input_key_pressed(int key) {
    if (key >= 0 && key < 512) {
        return g_input.keys[key] && !g_input.keys_prev[key];
    }
    return false;
}

// 检查按键是否刚刚释放
bool input_key_released(int key) {
    if (key >= 0 && key < 512) {
        return !g_input.keys[key] && g_input.keys_prev[key];
    }
    return false;
}

// 检查鼠标按钮是否按下
bool input_mouse_down(int button) {
    if (button >= 0 && button < 8) {
        return g_input.mouse_buttons[button];
    }
    return false;
}

// 检查鼠标按钮是否刚刚按下
bool input_mouse_pressed(int button) {
    if (button >= 0 && button < 8) {
        return g_input.mouse_buttons[button] && !g_input.mouse_buttons_prev[button];
    }
    return false;
}

// 检查鼠标按钮是否刚刚释放
bool input_mouse_released(int button) {
    if (button >= 0 && button < 8) {
        return !g_input.mouse_buttons[button] && g_input.mouse_buttons_prev[button];
    }
    return false;
}

// 获取鼠标位置
vec3 input_get_mouse_position() {
    return g_input.mouse_position;
}

// 获取鼠标增量
vec3 input_get_mouse_delta() {
    return g_input.mouse_delta;
}

// 获取鼠标滚轮增量
vec3 input_get_mouse_scroll() {
    return g_input.mouse_scroll;
}

// 设置鼠标锁定
void input_set_mouse_locked(bool locked) {
    g_input.mouse_locked = locked;
    
    if (locked) {
        glfwSetInputMode(g_input.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        // 获取当前鼠标位置作为锁定位置
        double x, y;
        glfwGetCursorPos(g_input.window, &x, &y);
        g_input.mouse_lock_pos = (vec3){(float)x, (float)y};
    } else {
        glfwSetInputMode(g_input.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        
        // 重置增量
        g_input.mouse_delta = (vec3){0, 0, 0};
    }
}

// 切换鼠标锁定
void input_toggle_mouse_locked() {
    input_set_mouse_locked(!g_input.mouse_locked);
}

// 检查鼠标是否锁定
bool input_is_mouse_locked() {
    return g_input.mouse_locked;
}

// 设置鼠标位置
void input_set_mouse_position(float x, float y) {
    if (g_input.window) {
        glfwSetCursorPos(g_input.window, x, y);
        g_input.mouse_position = (vec3){x, y};
        
        if (g_input.mouse_locked) {
            g_input.mouse_lock_pos = g_input.mouse_position;
        }
    }
}

// 显示/隐藏鼠标光标
void input_set_cursor_visible(bool visible) {
    if (g_input.window) {
        glfwSetInputMode(g_input.window, GLFW_CURSOR, 
                        visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}

// 获取窗口尺寸
vec3 input_get_window_size() {
    int width, height;
    glfwGetWindowSize(g_input.window, &width, &height);
    return (vec3){(float)width, (float)height};
}

// 获取窗口中心
vec3 input_get_window_center() {
    vec3 size = input_get_window_size();
    return (vec3){size.x * 0.5f, size.y * 0.5f, 0.0f};
}

// 检查窗口是否获得焦点
bool input_window_focused() {
    return glfwGetWindowAttrib(g_input.window, GLFW_FOCUSED);
}

// 获取按下的按键列表（用于调试）
void input_get_pressed_keys(int* keys, int max_keys) {
    int count = 0;
    for (int i = 0; i < 512 && count < max_keys; i++) {
        if (g_input.keys[i]) {
            keys[count++] = i;
        }
    }
    
    if (count < max_keys) {
        keys[count] = -1;
    }
}

// 打印输入状态（调试用）
void input_print_state() {
    printf("Input State:\\n");
    printf("  Mouse: (%.1f, %.1f) delta: (%.1f, %.1f)\\n",
           g_input.mouse_position.x, g_input.mouse_position.y,
           g_input.mouse_delta.x, g_input.mouse_delta.y);
    
    printf("  Scroll: (%.1f, %.1f)\\n",
           g_input.mouse_scroll.x, g_input.mouse_scroll.y);
    
    printf("  Mouse buttons: ");
    for (int i = 0; i < 3; i++) {
        if (g_input.mouse_buttons[i]) {
            printf("%d ", i);
        }
    }
    printf("\\n");
    
    printf("  Keys pressed: ");
    for (int i = 0; i < 512; i++) {
        if (g_input.keys[i]) {
            const char* name = glfwGetKeyName(i, 0);
            if (name) {
                printf("%s ", name);
            } else {
                printf("%d ", i);
            }
        }
    }
    printf("\\n");
}
