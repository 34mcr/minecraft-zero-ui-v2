#ifdef WINDOW_H
#define WINDOW_H


#include <stdbool.h>

typedef struct {
    int width;
    int height;
    const char* title;
    void* handle;
} Window;

Window* window_create(int width, int hight, const char* title);
void window_destroy(Window* window);
bool window_should_close(Window* window);
void window_poll_events();
void window_swap_buffers(Window* window);

#endif