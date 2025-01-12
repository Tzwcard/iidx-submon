#include "iidx-sub-gui-keypad.h"

#define FILL_DRAW_LIST(p, v) for (int i = 0; i < _countof(p); i++) { v.push_back(ImVec2(_x + _w / 2 + p[i].x * _h, _y + p[i].y * _h)); }
#define ADD_KEYPAD_BTN(V, T, PLAYER, IDX, C, W, _x, _y, _w, _h) T.side = PLAYER & 1; \
T.idx = IDX; \
T.check[0] = ImVec2( \
	C.x - W / 2, \
	C.y - W / 2 \
); \
T.check[1] = ImVec2( \
	C.x + W / 2, \
	C.y + W / 2 \
); \
T.draw[0] = ImVec2(T.check[0].x + _x, T.check[0].y + _y); \
T.draw[1] = ImVec2(T.check[1].x + _x, T.check[1].y + _y); \
V.push_back(T); \

static const auto COL_GREY = IM_COL32(128, 128, 128, 255),
COL_WHITE = IM_COL32(255, 255, 255, 255),
COL_BLACK = IM_COL32(0, 0, 0, 255);

static const uint16_t lookup_table[] = {
	0x0800, 0x8000, 0x0008,
	0x0400, 0x4000, 0x0004,
	0x0200, 0x2000, 0x0002,
	0x0100, 0x1000, 0x0001,
};

static const char lookup_charas[][3] = {
	"7", "8", "9",
	"4", "5", "6",
	"1", "2", "3",
	"0", "00", "",
};

int _KEYPAD::init(ImVec2 pos, ImVec2 size, float _line) {
	float _x = pos.x, _y = pos.y, _w = size.x, _h = size.y;
	line = _line > 1.f ? _line : 1.f;

	float _check_box_width = _h * 0.15f;
	float _space_to_side = _w * 0.1f,
		_space_to_top = _h * 0.125f + _check_box_width / 2,
		_space_btn = _check_box_width * 4 / 3;

	ctx_keypad_draw_btn tmp;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 12; j++) {
			ADD_KEYPAD_BTN(_btn, tmp, i & 1, j, ImVec2(
				_space_to_side + _space_btn * (int)(j % 3) + (i ? (_w - 2 * _space_to_side - 2 * _space_btn) : 0),
				_space_to_top + _space_btn * (int)(j / 3)
			), _check_box_width, _x, _y, _w, _h);
		}

	}

	frame[0][0] = ImVec2(
		_space_to_side - _check_box_width * 5 / 6 + _x,
		_space_to_top - _check_box_width * 5 / 6 + _y
	);
	frame[0][1] = ImVec2(
		frame[0][0].x + (3 + 4 / 3.f) * _check_box_width,
		frame[0][0].y + (4 + 5 / 3.f) * _check_box_width
	);

	frame[1][0] = ImVec2(
		_space_to_side - _check_box_width * 5 / 6 + (_w - 2 * _space_to_side - 2 * _space_btn) + _x,
		_space_to_top - _check_box_width * 5 / 6 + _y
	);
	frame[1][1] = ImVec2(
		frame[1][0].x + (3 + 4 / 3.f) * _check_box_width,
		frame[1][0].y + (4 + 5 / 3.f) * _check_box_width
	);

	_font_size = size.x * 0.008333f * 4;

	return 1;
}

int _KEYPAD::draw(ImDrawList* drawList) {
#if 0
	for (int i = 0; i < _countof(_cabinet_draw); i++) {
		if (_cabinet_draw[i]->size() >= 3) {
			drawList->AddConvexPolyFilled(
				_cabinet_draw[i]->data(),
				static_cast<int>(_cabinet_draw[i]->size()),
				COL_BLACK
			);

			drawList->AddPolyline(_cabinet_draw[i]->data(),
				static_cast<int>(_cabinet_draw[i]->size()),
				COL_GREY,
				true /* closed */,
				line
			);
		}
	}
#endif

	drawList->AddRect(
		frame[0][0],
		frame[0][1],
		COL_WHITE,
		4.f,
		0,
		1.f
	);

	drawList->AddRect(
		frame[1][0],
		frame[1][1],
		COL_WHITE,
		4.f,
		0,
		1.f
	);

	for (auto const& a : _btn) {
		drawList->AddRectFilled(
			a.draw[0],
			a.draw[1],
			COL_WHITE,
			0,
			0
		);

		drawList->AddRectFilled(
			ImVec2(a.draw[0].x + 2 * line, a.draw[0].y + 2 * line),
			ImVec2(a.draw[1].x - 2 * line, a.draw[1].y - 2 * line),
			COL_BLACK,
			0,
			0
		);
		drawList->AddRectFilled(
			ImVec2(a.draw[0].x + 4 * line, a.draw[0].y + 4 * line),
			ImVec2(a.draw[1].x - 4 * line, a.draw[1].y - 4 * line),
			(lookup_table[a.idx % 12] & keypad[a.side & 1]) ? COL_WHITE : COL_BLACK,
			0,
			0
		);

		if (_font) {
			ImVec2 sz = _font->CalcTextSizeA(_font_size, FLT_MAX, -1.0f, lookup_charas[a.idx % 12], NULL, NULL);
			drawList->AddText(
				_font,
				_font_size,
				ImVec2((a.draw[0].x + a.draw[1].x - sz.x) / 2,
					(a.draw[0].y + a.draw[1].y - sz.y) / 2),
				(lookup_table[a.idx % 12] & keypad[a.side & 1]) ? COL_BLACK : COL_WHITE,
				lookup_charas[a.idx % 12]
			);
		}
		else {
			drawList->AddText(
				_font,
				_font_size,
				ImVec2((a.draw[0].x + a.draw[1].x - _font_size * strlen(lookup_charas[a.idx % 12]) / 2) / 2,
					(a.draw[0].y + a.draw[1].y - _font_size) / 2),
				(lookup_table[a.idx % 12] & keypad[a.side & 1]) ? COL_BLACK : COL_WHITE,
				lookup_charas[a.idx % 12]
			);
		}

	}

	return 1;
}

int _KEYPAD::fetch_touch(ImVec2 pos, IIDX_SUBMON::E_TOUCH_STATE state) {
	if (state == IIDX_SUBMON::E_TOUCH_STATE::DOWN) {
		for (auto const& a : _btn) {
			if (pos.x >= a.check[0].x
				&& pos.y >= a.check[0].y
				&& pos.x <= a.check[1].x
				&& pos.y <= a.check[1].y
				) {
				keypad[a.side & 1] = lookup_table[a.idx % 12];
			}
		}
	}
	else if (state == IIDX_SUBMON::E_TOUCH_STATE::UP) {
		keypad[0] = 0;
		keypad[1] = 0;
	}

	return 1;
}