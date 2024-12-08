#pragma once
#include <imgui.h>
#include <vector>
#include "iidx-sub-gui-defs.h"

class _LED {
private:
	struct ctx_led_draw_btn {
		ImVec2 draw[2];
		ImVec2 check[2];
		uint32_t mask;
	};

	std::vector<ImVec2> play_panel,
		touch_panel,
		black_plate_l,
		black_plate_r,
		monitor,
		ceiling_left,
		ceiling_right,
		ceiling_back,
		title,
		base,
		led_panel_left,
		led_panel_right,
		cl_spk, cr_spk,
		spk_l, spk_r;

	std::vector<ImVec2>* _cabinet_draw[16] = {
		&base,
		&led_panel_left,
		&led_panel_right,
		&ceiling_back,
		&ceiling_left,
		&ceiling_right,
		&cl_spk, &cr_spk,
		&black_plate_l, &black_plate_r,
		&spk_l, &spk_r,
		&title,
		&monitor,
		&touch_panel,
		&play_panel
	};

	std::vector<ctx_led_draw_btn> _btn;

	uint32_t led_mask;
	float line;

public:
	_LED() : led_mask(0x1ffff), line(1.f) {};
	~_LED() = default;

	int init(ImVec2 pos, ImVec2 size, float line);
	int draw(ImDrawList* drawList);
	int fetch_touch(ImVec2 pos, IIDX_SUBMON::E_TOUCH_STATE state);

	uint32_t get(void) { return led_mask; }
};