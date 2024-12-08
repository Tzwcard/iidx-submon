#pragma once
#include <imgui.h>
#include <vector>
#include "iidx-sub-gui-defs.h"

class _RESIST {
private:
	float _val[2];
	ImVec2 _slider_draw_area[2][2];
	ImVec2 _frame[2][2];
	ImVec2 _slider_check_area[2][2];
	float _frame_width = 1.f;
	ImVec2 _slot = ImVec2(20.f, 100.f);

	float _unit = 20.f;
	float _space_width = 20.f;

	int _touch_active = -1;
	float _font_size = 10.f;

public:
	_RESIST() {
		_val[0] = .5f;
		_val[1] = .5f;

		_touch_active = -1;
	};
	~_RESIST() = default;

	int init(ImVec2 pos, ImVec2 size, float line);
	int draw(ImDrawList* drawList);
	unsigned char get(int id);

	int fetch_touch(ImVec2 pos, IIDX_SUBMON::E_TOUCH_STATE state);
};