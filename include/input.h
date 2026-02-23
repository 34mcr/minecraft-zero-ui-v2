#ifndef INPUT_H
#define INPUT_H

#include "math/vec3.h"
#include <stdbool.h>

typedef enum {
    KEY_UNKNOWN = -1,
    KEY_SPACE = 32,
    KEY_W = 87, KEY_A = 65, KEY_S = 83, KEY_D = 68,
    KEY_E = 69, KEY_Q = 81,
    KEY_LEFT_SHIFT = 340,
    KEY_LEFT_CONTROL = 341,
    KEY_ESCAPE = 256,
    KEY_F1 = 290, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
    KEY_1 = 49, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_6 = 54, KEY_7, KEY_8, KEY_9, KEY_0
} KeyCode;

typedef enum {
    MOUSE_BUTTON_LEFT = 0,
    MOUSE_BUTTON_RIGHT = 1,
    MOUSE_BUTTON_MIDDLE = 2
} MouseButton;

void input_init(void* window);
void input_update();
bool input_key_down(int key);
bool input_key_pressed(int key);
bool input_mouse_down(int button);
bool input_mouse_pressed(int button);
vec3 input_get_mouse_position();
vec3 input_get_mouse_delta();

#endif
