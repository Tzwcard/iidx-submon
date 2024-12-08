#include "iidx-sub-gui-effector.h"

static const auto COL_GREY = IM_COL32(40, 40, 40, 255),
COL_WHITE = IM_COL32(255, 255, 255, 255),
COL_BLACK = IM_COL32(0, 0, 0, 255);

static const char* SLIDER_TEXT[0x10] = {
	"VEFX",
	"low-EQ",
	"hi-EQ",
	"filter",
	"play\nvolume"
};

int _EFFECTOR::init(ImVec2 pos, ImVec2 size, float line) {
	_unit = size.y / 9.f;
	_eff_width = size.x / 9.f;
	_frame_width = line;

	float space_left = _eff_width * 2,
		to_right = _eff_width * 7;

	_frame_pos[0] = ImVec2(
		pos.x + space_left,
		pos.y + _unit
	);

	_frame_pos[1] = ImVec2(
		pos.x + to_right,
		pos.y + _unit * 8.5f
	);

	ImVec2 _draw_start = _frame_pos[0];
	for (int i = 0; i < 5; i++) {
		float bar_sp_l = _eff_width / 11.f;
		float bar_sp_t = _unit;
		float bar_width = _eff_width * 0.43f;

		_slider_draw_area[i][0] = ImVec2(
			_draw_start.x + bar_sp_l,
			_draw_start.y + bar_sp_t
		);

		_slider_draw_area[i][1] = ImVec2(
			_draw_start.x + bar_sp_l + bar_width,
			_draw_start.y + bar_sp_t + 5 * _unit
		);

		_slider_check_area[i][0] = ImVec2(
			_draw_start.x + bar_sp_l - pos.x,
			_draw_start.y + bar_sp_t - pos.y
		);

		_slider_check_area[i][1] = ImVec2(
			_draw_start.x + bar_sp_l + bar_width - pos.x,
			_draw_start.y + bar_sp_t + 5 * _unit - pos.y
		);

		_draw_start.x += _eff_width;
	}

	_font_size = size.x * 0.008333f;

	return 1;
}

char _EFFECTOR::get(int id) {
	if (id < 0 || id > 4) {
		return 0;
	}
	float _flt = _val[id];

	if (_flt > 1.f) _flt = 1.f;
	else if (_flt < 0.f) _flt = 0.f;

	return (char)(_flt * 15 + 0.5f);
}

int _EFFECTOR::draw(ImDrawList* drawList) {
	drawList->AddRectFilled(_frame_pos[0], _frame_pos[1], COL_WHITE, 4.f);

	drawList->AddRectFilled(
		ImVec2(_frame_pos[0].x + _frame_width, _frame_pos[0].y + _frame_width),
		ImVec2(_frame_pos[1].x - _frame_width, _frame_pos[1].y - _frame_width - _unit * 1.f),
		COL_BLACK,
		4.f
	);

	ImVec2 _draw_start = _frame_pos[0];
	for (int i = 0; i < 5; i++) {
		float space_left = _eff_width / 11.f;
		float bar_width = _eff_width * 0.43f;

		// TEXT
		drawList->AddText(
			NULL,
			i == 4 ? _font_size : 2 * _font_size,
			ImVec2(_draw_start.x + space_left, _draw_start.y + 0.5f * _unit),
			COL_WHITE,
			SLIDER_TEXT[i]
		);

		// BAR
		float space_bar_top = _unit;
		drawList->AddRect(
			_slider_draw_area[i][0],
			_slider_draw_area[i][1],
			COL_WHITE
		);

		// LINES
		for (int j = 0; j < 5; j++) {
			bool is_thick_line = false;

			if (i < 3) is_thick_line = j == 2;
			else is_thick_line = j == 0;

			drawList->AddLine(
				ImVec2(
					_slider_draw_area[i][1].x,
					_slider_draw_area[i][0].y + (j + 0.5f) * _unit + (is_thick_line ? -1.f : 0)
				),
				ImVec2(
					_slider_draw_area[i][1].x + (0.2f + (is_thick_line ? .1f : 0)) * _eff_width,
					_slider_draw_area[i][0].y + (j + 0.5f) * _unit + (is_thick_line ? -1.f : 0)
				),
				COL_WHITE,
				is_thick_line ? 3.f : 1.f
			);
		}

		// SLIDER
		float _flt = _val[i];

		if (_flt > 1.f) _flt = 1.f;
		else if (_flt < 0.f) _flt = 0.f;

		_flt = 1.f - _flt;

		drawList->AddRect(
			ImVec2(
				_slider_draw_area[i][0].x,
				_slider_draw_area[i][0].y + _flt * 4 * _unit
			),
			ImVec2(
				_slider_draw_area[i][1].x,
				_slider_draw_area[i][0].y + (_flt * 4 + 1) * _unit
			),
			COL_WHITE,
			4.f,
			0,
			3.f
		);

		float ___line_length = _slider_draw_area[i][1].x - _slider_draw_area[i][0].x;
		drawList->AddLine(
			ImVec2(
				(_slider_draw_area[i][0].x + _slider_draw_area[i][1].x) / 2.f - 0.3f * ___line_length,
				_slider_draw_area[i][0].y + (_flt * 4 + .5f) * _unit
			),
			ImVec2(
				(_slider_draw_area[i][0].x + _slider_draw_area[i][1].x) / 2.f + 0.3f * ___line_length,
				_slider_draw_area[i][0].y + (_flt * 4 + .5f) * _unit
			),
			COL_WHITE
		);

		// SPLITER
		if (i < 4) {
			drawList->AddLine(
				ImVec2(_draw_start.x + _eff_width, _draw_start.y + .5f * _unit),
				ImVec2(_draw_start.x + _eff_width, _draw_start.y + space_bar_top + 5 * _unit),
				COL_WHITE
			);
		}

		// PASS TO NEXT
		_draw_start.x += _eff_width;
	}

	return 1;
}

int _EFFECTOR::fetch_touch(ImVec2 pos, IIDX_SUBMON::E_TOUCH_STATE state) {
	if (state == IIDX_SUBMON::E_TOUCH_STATE::DOWN) {
		_touch_active = -1;
		for (int i = 0; i < 5; i++) {
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

		_flt = (pos.y - (_slider_check_area[_touch_active][0].y + 0.5f * _unit))
			/ (_slider_check_area[_touch_active][1].y - _slider_check_area[_touch_active][0].y - _unit);

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