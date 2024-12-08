#pragma once
#include <windows.h>
#include <stdint.h>

// #define DEBUG_BUILD

#ifdef DEBUG_BUILD
#define DEBUG_BUILD_WARNING_INFO "PLEASE REMOVE 'DEBUG_BUILD' DEFINE FROM FILE 'iidx-sub-gui.h'\n"
#endif

int iidx_sub_gui(void);

bool iidx_sub_gui_set_16seg(unsigned char*);
unsigned char iidx_sub_gui_get_resist(char);
char iidx_sub_gui_get_slider(char);
uint32_t iidx_sub_gui_get_led_mask(void);

void get_touch_info(HWND gui_hwnd, MSG* msg);