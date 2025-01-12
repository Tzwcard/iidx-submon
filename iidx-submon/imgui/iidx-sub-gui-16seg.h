#pragma once
#include <imgui.h>
#include <vector>

class _16SEG {
private:
	float _x, _y, _w, _h;
	std::vector<ImVec2> a1, a2, b, c, d1, d2, e, f, g1, g2, h, i, j, k, l, m;
	ImVec2 dp;
	float r_dp;

	std::vector<std::vector<ImVec2>*> _draw = {
		&a1, &a2, &b, &c, &d2, &d1, &e, &f, &h, &i, &j, &g2, &m, &l, &k, &g1,
	};
	bool _draw_red[17] = { 0 };

public:
	_16SEG() = default;
	~_16SEG() {
		_draw.clear();
	}

	int init(ImVec4);
	int set_draw(uint32_t);
	int set_draw_ch(unsigned char);

	int draw(ImDrawList*);

};