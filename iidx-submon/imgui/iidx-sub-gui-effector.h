#pragma once
#include <imgui.h>
#include <vector>
#include "iidx-sub-gui-defs.h"

class _EFFECTOR {
private:
	float _val[5];
	ImVec2 _slider_draw_area[5][2];
	ImVec2 _slider_check_area[5][2];
	ImVec2 _frame_pos[2] = { ImVec2(100.f, 100.f), ImVec2(200.f, 200.f) };
	float _frame_width = 1.f;
	ImVec2 _slot = ImVec2(20.f, 100.f);

	float _unit = 20.f;
	float _eff_width = 20.f;

	int _touch_active = -1;
	float _font_size = 10.f;

public:
	_EFFECTOR() {
		_val[0] = .5f;
		_val[1] = .5f;
		_val[2] = .5f;
		_val[3] = 1.f;
		_val[4] = 1.f;

		_touch_active = -1;
	};
	~_EFFECTOR() = default;

	int init(ImVec2 pos, ImVec2 size, float line);
	int draw(ImDrawList* drawList);
	char get(int id);

	int fetch_touch(ImVec2 pos, IIDX_SUBMON::E_TOUCH_STATE state);
};