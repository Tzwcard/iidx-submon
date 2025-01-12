#pragma once
#include <imgui.h>
#include <vector>
#include "iidx-sub-gui-defs.h"

class _KEYPAD {
private:
	struct ctx_keypad_draw_btn {
		ImVec2 draw[2];
		ImVec2 check[2];
		uint8_t side;
		int idx;
	};

	ImVec2 frame[2][2];

	std::vector<ctx_keypad_draw_btn> _btn;

	uint16_t keypad[2];
	float line;

	ImFont* _font;
	float _font_size;

public:
	_KEYPAD() : keypad{ 0 }, line(1.f), _font_size(13.f), _font(NULL) {};
	~_KEYPAD() = default;

	void setfont(void* a) {
		if (a) _font = (ImFont*)a;
	}

	int init(ImVec2 pos, ImVec2 size, float line);
	int draw(ImDrawList* drawList);
	int fetch_touch(ImVec2 pos, IIDX_SUBMON::E_TOUCH_STATE state);

	uint16_t get(char a) { return _byteswap_ushort(keypad[a]); }
};