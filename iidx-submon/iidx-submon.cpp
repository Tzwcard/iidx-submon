#include "pch.h"
#include "imgui/imgui-win32-opengl3.h"
#include "imgui/iidx-sub-gui.h"
#include <iostream>
#include <thread>

static std::thread _th_gui;

/*
 * Init submon module
 * return 1 on success, return 0 or less if failed
 */
int iidx_submon_init(void) {
	/*
	   Because we can only create one D3DDevice at a time
	   that means without hooking to bm2dx.exe/bm2dx.dll
	   we cannot use the d3ddev to draw the second window
	   so we use OpenGL backend here.

	   Plain HDC drawing is fun, and raw OpenGL functions
	   are fun too, but I use imgui here since it's simple
	   to use.
	 */
	int ret = gui_init();

	if (ret <= 0) {
		printf("gui_init() returns %d\n", ret);
		return ret;
	}

	_th_gui = std::thread(gui_main);
	printf("gui_main() start\n");

	return 1;
}

/*
 * Get turntable resistance value for player #p (0, 1)
 * return corresponding resistance set from gui (0 - 255)
 */
unsigned char iidx_submon_get_resist(char p) {
	return iidx_sub_gui_get_resist(p);
}

/*
 * Get slider value for slider #id (0 - 4)
 * return corresponding slider value set from gui (0 - 15)
 */
char iidx_submon_get_slider(char id) {
	return iidx_sub_gui_get_slider(id);
}

/*
 * Get led area mask
 * return mask value set from gui (0x00000 - 0x1FFFF)
 */
uint32_t iidx_submon_get_led_mask(void) {
	return iidx_sub_gui_get_led_mask();
}

/*
 * Set 16-segment string
 * will do a memcpy of 9 unsigned char
 * always return true
 */
bool iidx_submon_set_16seg(unsigned char* seg) {
	return iidx_sub_gui_set_16seg(seg);
}

/*
 * Finalize submon module
 */
void iidx_submon_finl(void) {
	gui_close();
	if (_th_gui.joinable()) {
		_th_gui.join();
	}
}