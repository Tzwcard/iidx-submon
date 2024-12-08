#include "iidx-sub-gui-16seg.h"
#include "iidx-16seg-def.h"

static const auto COL_GREY = IM_COL32(20, 20, 20, 255), COL_RED = IM_COL32(255, 0, 0, 255);
static const std::vector<ImVec2>
_base_a1 = { ImVec2(0.302941f, 0.107333f), ImVec2(0.337255f, 0.085333f), ImVec2(0.558824f, 0.085333f), ImVec2(0.583333f, 0.106000f), ImVec2(0.515686f, 0.147000f), ImVec2(0.350000f, 0.147000f), },
_base_a2 = { ImVec2(0.613725f, 0.109333f), ImVec2(0.650000f, 0.085333f), ImVec2(0.873529f, 0.085333f), ImVec2(0.897059f, 0.106667f), ImVec2(0.825490f, 0.147000f), ImVec2(0.657843f, 0.147000f), },
_base_b  = { ImVec2(0.909804f, 0.116000f), ImVec2(0.837255f, 0.156000f), ImVec2(0.755882f, 0.459333f), ImVec2(0.793137f, 0.488667f), ImVec2(0.848039f, 0.458000f), ImVec2(0.932353f, 0.134667f), },
_base_c  = { ImVec2(0.791176f, 0.507333f), ImVec2(0.739216f, 0.535333f), ImVec2(0.660784f, 0.838667f), ImVec2(0.708824f, 0.878667f), ImVec2(0.746078f, 0.857333f), ImVec2(0.828431f, 0.535333f), },
_base_d1 = { ImVec2(0.101961f, 0.890667f), ImVec2(0.172549f, 0.849000f), ImVec2(0.338235f, 0.849000f), ImVec2(0.386275f, 0.886667f), ImVec2(0.356863f, 0.910000f), ImVec2(0.121569f, 0.910000f), },
_base_d2 = { ImVec2(0.413725f, 0.889333f), ImVec2(0.482353f, 0.849000f), ImVec2(0.647059f, 0.849000f), ImVec2(0.695098f, 0.886000f), ImVec2(0.661765f, 0.910000f), ImVec2(0.438235f, 0.910000f), },
_base_e  = { ImVec2(0.201961f, 0.506667f), ImVec2(0.146078f, 0.537333f), ImVec2(0.065686f, 0.860000f), ImVec2(0.091176f, 0.880000f), ImVec2(0.159804f, 0.841333f), ImVec2(0.238235f, 0.536000f), },
_base_f  = { ImVec2(0.287255f, 0.115333f), ImVec2(0.252941f, 0.135333f), ImVec2(0.167647f, 0.460000f), ImVec2(0.203922f, 0.489333f), ImVec2(0.256863f, 0.458667f), ImVec2(0.333333f, 0.154667f), },
_base_g1 = { ImVec2(0.217647f, 0.499333f), ImVec2(0.271569f, 0.467000f), ImVec2(0.427451f, 0.467000f), ImVec2(0.487255f, 0.497333f), ImVec2(0.410784f, 0.528000f), ImVec2(0.253922f, 0.528000f), },
_base_g2 = { ImVec2(0.506863f, 0.498000f), ImVec2(0.586275f, 0.467000f), ImVec2(0.740196f, 0.467000f), ImVec2(0.778431f, 0.496667f), ImVec2(0.726471f, 0.528000f), ImVec2(0.566667f, 0.528000f), },
_base_h  = { ImVec2(0.341176f, 0.162000f), ImVec2(0.321569f, 0.234000f), ImVec2(0.433333f, 0.462667f), ImVec2(0.491176f, 0.488000f), ImVec2(0.467647f, 0.344667f), ImVec2(0.397059f, 0.206667f), },
_base_i  = { ImVec2(0.596078f, 0.113333f), ImVec2(0.544118f, 0.143333f), ImVec2(0.479412f, 0.388000f), ImVec2(0.500000f, 0.482000f), ImVec2(0.570588f, 0.382667f), ImVec2(0.631373f, 0.145333f), },
_base_j  = { ImVec2(0.825490f, 0.164000f), ImVec2(0.730392f, 0.216000f), ImVec2(0.602941f, 0.340667f), ImVec2(0.511765f, 0.485333f), ImVec2(0.587255f, 0.460000f), ImVec2(0.804902f, 0.247333f), },
_base_k  = { ImVec2(0.484314f, 0.508000f), ImVec2(0.405882f, 0.536667f), ImVec2(0.190196f, 0.747333f), ImVec2(0.172549f, 0.831333f), ImVec2(0.262745f, 0.780000f), ImVec2(0.387255f, 0.658667f), },
_base_l  = { ImVec2(0.492157f, 0.512000f), ImVec2(0.420588f, 0.614667f), ImVec2(0.363725f, 0.849333f), ImVec2(0.400980f, 0.879333f), ImVec2(0.451961f, 0.851333f), ImVec2(0.513726f, 0.610667f), },
_base_m  = { ImVec2(0.503922f, 0.505333f), ImVec2(0.526471f, 0.650667f), ImVec2(0.592157f, 0.786667f), ImVec2(0.651961f, 0.832000f), ImVec2(0.673529f, 0.761333f), ImVec2(0.559804f, 0.532000f), };

int _16SEG::init(ImVec4 _coord) {
	_x = _coord.x;
	_y = _coord.y;
	_w = _coord.z;
	_h = _coord.w;

	for (const auto& _it : _base_a1) { a1.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_a2) { a2.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_b) { b.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_c) { c.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_d1) { d1.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_d2) { d2.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_e) { e.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_f) { f.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_g1) { g1.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_g2) { g2.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_h) { h.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_i) { i.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_j) { j.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_k) { k.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_l) { l.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }
	for (const auto& _it : _base_m) { m.push_back(ImVec2(_it.x * _w + _x, _it.y * _h + _y)); }

	dp.x = 0.9f * _w + _x;
	dp.y = 0.877333f * _h + _y;
	r_dp = 0.092804f * _w / 2.f;

	return 1;
}

int _16SEG::set_draw(uint32_t flg) {
	for (int it = 0; it < 17; it++) {
		_draw_red[it] = flg & (1 << it);
	}
	return 1;
}

int _16SEG::set_draw_ch(unsigned char c) {
	uint32_t f = IIDX_16SEG_TABLE[c];

	return set_draw(f);
}

int _16SEG::draw(ImDrawList * drawList) {
	int pos = 0;

	for (const auto& points : _draw) {
		if (points->size() >= 3) {
			drawList->AddConvexPolyFilled(
				points->data(), 
				static_cast<int>(points->size()),
				_draw_red[pos++] ? COL_RED : COL_GREY
			);
		}
	}

	drawList->AddCircleFilled(dp, r_dp, _draw_red[16] ? COL_RED : COL_GREY);

	return 1;
}