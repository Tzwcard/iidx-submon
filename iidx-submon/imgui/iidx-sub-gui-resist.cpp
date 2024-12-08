#include "iidx-sub-gui-resist.h"

static const auto COL_GREY = IM_COL32(40, 40, 40, 255),
COL_WHITE = IM_COL32(255, 255, 255, 255),
COL_BLACK = IM_COL32(0, 0, 0, 255),
COL_TRANSPARENT_BLACK = IM_COL32(0, 0, 0, 0xa0),
COL_TRANSPARENT_BRIGHT_BLUE = IM_COL32(0x1F, 0xA8, 0xFB, 0xa0);

int _RESIST::init(ImVec2 pos, ImVec2 size, float line) {
	_unit = size.y / 9.f;
	_space_width = size.x / 3.75f;
	_frame_width = line; 
	_font_size = size.x * 0.008333f;

	float bar_width = (size.x / 10.f) * 0.5f;

	_frame[0][0] = ImVec2(pos.x, pos.y + 1.f * _unit);
	_frame[0][1] = ImVec2(pos.x + size.x * 0.35f, pos.y + 8.f * _unit);

	_frame[1][0] = ImVec2(pos.x + size.x * 0.65f, pos.y + 1.f * _unit);
	_frame[1][1] = ImVec2(pos.x + size.x * 1.0f, pos.y + 8.f * _unit);

	_slider_draw_area[0][0] = ImVec2(
		pos.x + _space_width,
		pos.y + 1.5f * _unit
	);

	_slider_draw_area[0][1] = ImVec2(
		pos.x + _space_width + bar_width,
		pos.y + 7.5f * _unit
	);

	_slider_check_area[0][0] = ImVec2(
		_space_width,
		1.5f * _unit
	);

	_slider_check_area[0][1] = ImVec2(
		_space_width + bar_width,
		7.5f * _unit
	);

	_slider_draw_area[1][0] = ImVec2(
		pos.x + (size.x - _space_width - bar_width),
		pos.y + 1.5f * _unit
	);

	_slider_draw_area[1][1] = ImVec2(
		pos.x + (size.x - _space_width),
		pos.y + 7.5f * _unit
	);

	_slider_check_area[1][0] = ImVec2(
		size.x - _space_width - bar_width,
		1.5f * _unit
	);

	_slider_check_area[1][1] = ImVec2(
		size.x - _space_width,
		7.5f * _unit
	);

	return 1;
}

unsigned char _RESIST::get(int id) {
	if (id < 0 || id > 1) {
		return 0;
	}
	float _flt = _val[id];

	if (_flt > 1.f) _flt = 1.f;
	else if (_flt < 0.f) _flt = 0.f;

	return (unsigned char)(_flt * 255 + 0.5f);
}

int _RESIST::draw(ImDrawList* drawList) {

	for (int i = 0; i < 2; i++) {
		// Frame
		drawList->AddRect(
			_frame[i][0],
			_frame[i][1],
			COL_WHITE,
			4.f,
			0,
			1.f
		);

		// BAR
		float space_bar_top = _unit;
		drawList->AddRect(
			_slider_draw_area[i][0],
			_slider_draw_area[i][1],
			COL_WHITE,
			0.f,
			0,
			2.f
		);

		// SLIDER
		float _flt = _val[i];

		if (_flt > 1.f) _flt = 1.f;
		else if (_flt < 0.f) _flt = 0.f;

		float _flt_draw = 1.f - _flt;

		float y_h = _slider_draw_area[i][1].y - _slider_draw_area[i][0].y,
			y_beg = y_h * _flt_draw;

		if (_flt > 0.f) {
			drawList->AddRectFilledMultiColor(
				ImVec2(_slider_draw_area[i][0].x, _slider_draw_area[i][0].y + y_beg),
				_slider_draw_area[i][1],
				IM_COL32(0, 0x66 + 0x66 * _flt_draw, 0xFF, 0xa0),
				IM_COL32(0, 0x66 + 0x66 * _flt_draw, 0xFF, 0xa0),
				IM_COL32(0, 0xCC, 0xFF, 0xa0),
				IM_COL32(0, 0xCC, 0xFF, 0xa0)
			);

			drawList->AddLine(
				ImVec2(_slider_draw_area[i][0].x, _slider_draw_area[i][0].y + y_beg),
				ImVec2(_slider_draw_area[i][1].x, _slider_draw_area[i][0].y + y_beg + 1.f),
				COL_WHITE
			);
		}
	}

	return 1;
}

int _RESIST::fetch_touch(ImVec2 pos, IIDX_SUBMON::E_TOUCH_STATE state) {
	if (state == IIDX_SUBMON::E_TOUCH_STATE::DOWN) {
		_touch_active = -1;
		for (int i = 0; i < 2; i++) {
			if (pos.x >= _slider_check_area[i][0].x
				&& pos.y >= _slider_check_area[i][0].y
				&& pos.x <= _slider_check_area[i][1].x
				&& pos.y <= _slider_check_area[i][1].y
				) {
				_touch_active = i;
				break;
			}
		}
	}
	else if (state == IIDX_SUBMON::E_TOUCH_STATE::UP) {
		_touch_active = -1;
	}

	if (_touch_active != -1) {
		float _flt = 0.f;

		_flt = (pos.y - _slider_check_area[_touch_active][0].y)
			/ (_slider_check_area[_touch_active][1].y - _slider_check_area[_touch_active][0].y);

		if (_flt > 1.f) {
			_flt = 1.f;
		}
		else if (_flt < 0.f) {
			_flt = 0.f;
		}

		_val[_touch_active] = 1.0f - _flt;
	}

	return 1;
}