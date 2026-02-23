#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

static GLFWwindow* g_window = NULL;
static int g_width = 800;
static int g_height = 600;
static const char* g_title = "Minecraft Clone";

// GLFW错误回调
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\\n", error, description);
}

Window* window_create(int width, int height, const char* title) {
    // 设置错误回调
    glfwSetErrorCallback(glfw_error_callback);
    
    // 初始化GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\\n");
        return NULL;
    }
    
    // 配置GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    // 创建窗口
    g_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!g_window) {
        fprintf(stderr, "Failed to create GLFW window\\n");
        glfwTerminate();
        return NULL;
    }
    
    // 设置为当前上下文
    glfwMakeContextCurrent(g_window);
    
    // 初始化GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\\n");
        glfwDestroyWindow(g_window);
        glfwTerminate();
        return NULL;
    }
    
    // 启用垂直同步
    glfwSwapInterval(1);
    
    g_width = width;
    g_height = height;
    g_title = title;
    
    printf("Window created: %dx%d \"%s\"\\n", width, height, title);
    return (Window*)g_window;
}

void window_destroy(Window* window) {
    if (g_window) {
        glfwDestroyWindow(g_window);
    }
    glfwTerminate();
    printf("Window destroyed\\n");
}

bool window_should_close(Window* window) {
    return glfwWindowShouldClose(g_window);
}

void window_poll_events() {
    glfwPollEvents();
}

void window_swap_buffers(Window* window) {
    glfwSwapBuffers(g_window);
}

void window_set_should_close(Window* window, bool value) {
    glfwSetWindowShouldClose(g_window, value);
}

int window_get_width(Window* window) {
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    return width;
}

int window_get_height(Window* window) {
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    return height;
}
