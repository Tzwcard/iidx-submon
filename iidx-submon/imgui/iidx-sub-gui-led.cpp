#include "iidx-sub-gui-led.h"

#define FILL_DRAW_LIST(p, v) for (int i = 0; i < _countof(p); i++) { v.push_back(ImVec2(_x + _w / 2 + p[i].x * _h, _y + p[i].y * _h)); }
#define ADD_LED_TOGGLE_BTN(V, T, MSK, C, L, W, _x, _y, _w, _h) T.mask = MSK; \
T.check[0] = ImVec2( \
	_w / 2 + (L ? C.x : -C.x) * _h - W / 2, \
	C.y * _h - W / 2 \
); \
T.check[1] = ImVec2( \
	_w / 2 + (L ? C.x : -C.x) * _h + W / 2, \
	C.y * _h + W / 2 \
); \
T.draw[0] = ImVec2(T.check[0].x + _x, T.check[0].y + _y); \
T.draw[1] = ImVec2(T.check[1].x + _x, T.check[1].y + _y); \
V.push_back(T); \

static const auto COL_GREY = IM_COL32(128, 128, 128, 255),
COL_WHITE = IM_COL32(255, 255, 255, 255),
COL_BLACK = IM_COL32(0, 0, 0, 255);

static const ImVec2
_d_pp[] = { ImVec2(-0.364130f, 0.836957f),ImVec2(0.364130f, 0.836957f),ImVec2(0.467391f, 0.943841f), ImVec2(0.436594f, 0.971014f),ImVec2(-0.436594f, 0.971014f), ImVec2(-0.467391f, 0.943841f), },
_d_tm[] = { ImVec2(-0.235507f, 0.692029f),ImVec2(0.235507f, 0.692029f), ImVec2(0.277174f, 0.838768f),ImVec2(-0.277174f, 0.838768f) },
_d_mn[] = { ImVec2(-0.234072f, 0.425725f),ImVec2(0.234072f, 0.425725f),ImVec2(0.234072f, 0.697217f),ImVec2(-0.234072f, 0.697217f) },
_d_cl[] = { ImVec2(-0.226449f, 0.230072f),ImVec2(-0.164855f, 0.230072f),ImVec2(-0.126812f, 0.278986f),ImVec2(-0.126812f, 0.443913f),ImVec2(-0.152174f, 0.443913f),ImVec2(-0.152174f, 0.306159f), },
_d_cr[] = { ImVec2(0.126812f, 0.278986f),ImVec2(0.164855f, 0.230072f),ImVec2(0.226449f, 0.230072f),ImVec2(0.152174f, 0.306159f),ImVec2(0.152174f, 0.443913f),ImVec2(0.126812f, 0.443913f), },
_d_tt[] = { ImVec2(-0.443841f, 0.045290f),ImVec2(0.443841f, 0.045290f),ImVec2(0.505435f, 0.144928f),ImVec2(0.456522f, 0.237319f),ImVec2(-0.456522f, 0.237319f),ImVec2(-0.505435f, 0.144928f) },
_d_ll[] = { ImVec2(-0.630435f, 0.152174f),ImVec2(-0.668478f, 0.195652f),ImVec2(-0.393116f, 0.855072f),ImVec2(-0.329710f, 0.793478f),ImVec2(-0.329710f, 0.750000f),ImVec2(-0.413043f, 0.481884f),ImVec2(-0.413043f, 0.416667f),ImVec2(-0.474638f, 0.221014f), },
_d_lr[] = { ImVec2(0.630435f, 0.152174f),ImVec2(0.668478f, 0.195652f),ImVec2(0.393116f, 0.855072f),ImVec2(0.329710f, 0.793478f),ImVec2(0.329710f, 0.750000f),ImVec2(0.413043f, 0.481884f),ImVec2(0.413043f, 0.416667f),ImVec2(0.474638f, 0.221014f), },
_d_s0[] = { ImVec2(-0.204710f, 0.329710f),ImVec2(-0.188406f, 0.318841f),ImVec2(-0.094203f, 0.318841f),ImVec2(-0.092391f, 0.364130f),ImVec2(-0.096014f, 0.391304f),ImVec2(-0.195652f, 0.391304f), },
_d_s1[] = { ImVec2(0.204710f, 0.329710f),ImVec2(0.188406f, 0.318841f),ImVec2(0.094203f, 0.318841f),ImVec2(0.092391f, 0.364130f),ImVec2(0.096014f, 0.391304f),ImVec2(0.195652f, 0.391304f), },
_d_cb[] = { ImVec2(-0.295290f, 0.222826f),ImVec2(-0.226449f, 0.280797f),ImVec2(0.226449f, 0.280797f),ImVec2(0.211957f, 0.346014f),ImVec2(-0.211957f, 0.346014f),ImVec2(-0.210145f, 0.431159f),ImVec2(0.210145f, 0.431159f),ImVec2(0.211957f, 0.346014f),ImVec2(-0.211957f, 0.346014f),ImVec2(-0.226449f, 0.280797f),ImVec2(0.226449f, 0.280797f),ImVec2(0.295290f, 0.222826f), },
_d_bl[] = { ImVec2(-0.353261f, 0.277174f),ImVec2(-0.365942f, 0.297101f),ImVec2(-0.329710f, 0.467391f),ImVec2(-0.320652f, 0.476449f),ImVec2(-0.228261f, 0.472826f),ImVec2(-0.257246f, 0.280797f), },
_d_br[] = { ImVec2(0.353261f, 0.277174f),ImVec2(0.365942f, 0.297101f),ImVec2(0.329710f, 0.467391f),ImVec2(0.320652f, 0.476449f),ImVec2(0.228261f, 0.472826f),ImVec2(0.257246f, 0.280797f), },
_d_sl[] = { ImVec2(-0.311594f, 0.496377f),ImVec2(-0.307971f, 0.599638f),ImVec2(-0.242754f, 0.594203f),ImVec2(-0.244565f, 0.496377f), },
_d_sr[] = { ImVec2(0.311594f, 0.496377f),ImVec2(0.307971f, 0.599638f),ImVec2(0.242754f, 0.594203f),ImVec2(0.244565f, 0.496377f), },
_d_bs[] = { ImVec2(-0.365942f, 0.958333f),ImVec2(-0.384058f, 1.f),ImVec2(0.384058f, 1.f),ImVec2(0.365942f, 0.958333f), };

static const ImVec2 _chk[]{ ImVec2(-0.135870f, 0.346014f), ImVec2(-0.465580f, 0.489130f), ImVec2(-0.201087f, 0.768116f) };

int _LED::init(ImVec2 pos, ImVec2 size, float _line) {
	float _x = pos.x, _y = pos.y, _w = size.x, _h = size.y;
	line = _line > 1.f ? _line : 1.f;

	FILL_DRAW_LIST(_d_pp, play_panel);
	FILL_DRAW_LIST(_d_tm, touch_panel);
	FILL_DRAW_LIST(_d_mn, monitor);
	FILL_DRAW_LIST(_d_cl, ceiling_left);
	FILL_DRAW_LIST(_d_cr, ceiling_right);
	FILL_DRAW_LIST(_d_tt, title);
	FILL_DRAW_LIST(_d_ll, led_panel_left);
	FILL_DRAW_LIST(_d_lr, led_panel_right);
	FILL_DRAW_LIST(_d_s0, cl_spk);
	FILL_DRAW_LIST(_d_s1, cr_spk);
	FILL_DRAW_LIST(_d_cb, ceiling_back);
	FILL_DRAW_LIST(_d_bl, black_plate_l);
	FILL_DRAW_LIST(_d_br, black_plate_r);
	FILL_DRAW_LIST(_d_sl, spk_l);
	FILL_DRAW_LIST(_d_sr, spk_r);
	FILL_DRAW_LIST(_d_bs, base);

	float _check_box_width = _h * 0.09f;

	ctx_led_draw_btn tmp;
	ADD_LED_TOGGLE_BTN(_btn, tmp, (1 << 5), _chk[0], 1, _check_box_width, _x, _y, _w, _h);
	ADD_LED_TOGGLE_BTN(_btn, tmp, (1 << 8), _chk[0], 0, _check_box_width, _x, _y, _w, _h);
	ADD_LED_TOGGLE_BTN(_btn, tmp, 0x03804, _chk[1], 1, _check_box_width, _x, _y, _w, _h);
	ADD_LED_TOGGLE_BTN(_btn, tmp, 0x1C008, _chk[1], 0, _check_box_width, _x, _y, _w, _h);
	ADD_LED_TOGGLE_BTN(_btn, tmp, (1 << 9), _chk[2], 1, _check_box_width, _x, _y, _w, _h);
	ADD_LED_TOGGLE_BTN(_btn, tmp, (1 << 10), _chk[2], 0, _check_box_width, _x, _y, _w, _h);

	return 1;
}

int _LED::draw(ImDrawList* drawList) {
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
			(a.mask & led_mask) ? COL_WHITE : COL_BLACK,
			0,
			0
		);
	}

	return 1;
}

int _LED::fetch_touch(ImVec2 pos, IIDX_SUBMON::E_TOUCH_STATE state) {
	if (state == IIDX_SUBMON::E_TOUCH_STATE::DOWN) {
		for (auto const& a : _btn) {
			if (pos.x >= a.check[0].x
				&& pos.y >= a.check[0].y
				&& pos.x <= a.check[1].x
				&& pos.y <= a.check[1].y
				) {
				led_mask ^= a.mask;
			}
		}
	}

	return 1;
}