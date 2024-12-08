#include "pch.h"
#include "imgui/imgui-win32-opengl3.h"
#include "imgui/iidx-sub-gui.h"
#include <iostream>
#include <thread>

static std::thread _th_gui;

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

unsigned char iidx_submon_get_resist(char p) {
	return iidx_sub_gui_get_resist(p);
}

char iidx_submon_get_slider(char id) {
	return iidx_sub_gui_get_slider(id);
}

bool iidx_submon_set_16seg(unsigned char* seg) {
	return iidx_sub_gui_set_16seg(seg);
}

uint32_t iidx_submon_get_led_mask(void) {
	return iidx_sub_gui_get_led_mask();
}

void iidx_submon_finl(void) {
	gui_close();
	if (_th_gui.joinable()) {
		_th_gui.join();
	}
}