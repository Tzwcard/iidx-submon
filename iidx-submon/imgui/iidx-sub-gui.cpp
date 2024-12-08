#include "iidx-sub-gui.h"

#include <gl/GL.h>
#include <imgui.h>
#include <string>
#include <vector>

#include "iidx-sub-gui-defs.h"

#include "iidx-sub-gui-16seg.h"
#include "iidx-sub-gui-effector.h"
#include "iidx-sub-gui-resist.h"
#include "iidx-sub-gui-led.h"

#ifdef DEBUG_BUILD
#define SIXTEEN_SEG_DEBUG
#define PRINT_DEBUG
#define MOUSE_DEBUG
#endif

struct ctx_iidx_iodata {
    // 16SEG
    unsigned char seg[10];

    // TURNTABLE RESISTANCE
    unsigned char resist[2];

    // SLIDER VALUES
    char slider[5];

    // LED MASK, 0x1FFFF ALL ON, 0x00000 ALL OFF
    uint32_t led_mask;

    // LED COLOR, RESERVED NOW
    uint32_t led_color;
};

static ctx_iidx_iodata _iodata = { 0 };

static int init_coord(ImGuiViewport* viewport);
static int gui_draw_16seg(void);
static int gui_draw_control_frame(void);
static int gui_draw_effector(void);
static int gui_draw_tt_resist(void);
static int gui_draw_led(void);
static int get_io(void);

static float _16seg_height = 0.f,
_control_width = 100.f,
_control_height = 100.f,
_line_width = 1.0f,
_width_16seg = 100.f;
static ImVec2 _pos_window, _sz_window;

static bool set_coord = false;
static std::vector<ImVec2> _frame[2];

enum class E_DISPLAY_TYPE {
    NONE,
    EFFECTOR,
    RESIST,
    LED_PANEL,
};

struct ctx_gui_btn_info {
    ImVec2 p_box_draw[2];
    ImVec2 p_box_check[2];
    const char* text;
    E_DISPLAY_TYPE type;
};

static E_DISPLAY_TYPE _display_type = E_DISPLAY_TYPE::NONE;
static std::vector<ctx_gui_btn_info> _btn;

/*************   TOUCH  *************/
static ImVec2 _touch_point;
static int _touch_state = 0;
static DWORD _touch_current = 0xffffffff;

/*************   16SEG  *************/
static _16SEG _chr[9];

/************* EFFECTOR *************/
static _EFFECTOR _eff;

/************* EFFECTOR *************/
static _RESIST _ttresist;

/************* EFFECTOR *************/
static _LED _led;

int iidx_sub_gui(void) {
    get_io();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    if (!set_coord) {
        init_coord(viewport);

        set_coord = true;
    }

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::SetNextWindowViewport(viewport->ID);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

    ImGui::SetWindowFontScale(4.f);

    ImGui::Begin("##IIDX_SUBMONITOR", nullptr, window_flags);

    ImGui::SetCursorPosY(_16seg_height);
    
#ifdef SIXTEEN_SEG_DEBUG
    int tick = (int)(ImGui::GetTime() * 2) % 0x60;
    for (int i = 0; i < 9; i++) {
        unsigned char pos = tick + 0x20 + i;
        _iodata.seg[i] = pos >= 0x80 ? pos - 0x60 : pos;
    }
#endif

#ifdef PRINT_DEBUG
    ImGui::Text("fps %.f, time %f", ImGui::GetIO().Framerate, ImGui::GetTime());

    ImGui::Text("SZ_WND: %dx%d, TOUCH_CURR: %d, pos (%f, %f)",
        (int)viewport->Size.x, (int)viewport->Size.y,
        _touch_state,
        _touch_point.x,
        _touch_point.y
    );
    ImGui::Text("SLIDER: %d %d %d %d %d, RESIST: %d %d, LED: %05x",
        _iodata.slider[0],
        _iodata.slider[1],
        _iodata.slider[2],
        _iodata.slider[3],
        _iodata.slider[4],
        _iodata.resist[0],
        _iodata.resist[1],
        _iodata.led_mask
    );
#endif

    gui_draw_16seg();
    
    switch (_display_type) {
    case E_DISPLAY_TYPE::EFFECTOR:
        gui_draw_effector();
        break;
    case E_DISPLAY_TYPE::RESIST:
        gui_draw_tt_resist();
        break;
    case E_DISPLAY_TYPE::LED_PANEL:
        gui_draw_led();
        break;
    }

    gui_draw_control_frame();

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    return 1;
}

/******************************************/
static int init_coord(ImGuiViewport* viewport) {
    int idx = 0;

    float width_16seg = viewport->Size.x / 9.f;
    float height_16seg = width_16seg / 1020.f * 1500.f;

    _width_16seg = width_16seg;

    for (int i = 0; i < 9; i++) {
        _chr[i].init(ImVec4(
            viewport->Pos.x + (viewport->Size.x * idx / 9.f),
            viewport->Pos.y,
            width_16seg,
            height_16seg
        ));

        idx++;
    }

    _16seg_height = height_16seg;

    _control_width = viewport->Size.x;
    _control_height = viewport->Size.y * .9f - _16seg_height;

    float control_btm = viewport->Size.y * .9f;

    _line_width = (viewport->Size.y / 1000.f) < 1.f ? 1.f : (viewport->Size.y / 1000.f);
    float triangle_len = (0.01f * _control_width) < 4.f ? 4.f : (0.01f * _control_width);

    _frame[0] = {
        ImVec2(0.f, _16seg_height),
        ImVec2(_control_width, _16seg_height),
        ImVec2(_control_width, _line_width + _16seg_height),
        ImVec2(triangle_len, _line_width + _16seg_height),
        ImVec2(0.f, triangle_len + _line_width + _16seg_height)
    };

    _frame[1] = {
        ImVec2(_control_width, control_btm),
        ImVec2(0.f, control_btm),
        ImVec2(0.f, control_btm - _line_width),
        ImVec2(_control_width - triangle_len, control_btm - _line_width),
        ImVec2(_control_width, control_btm - _line_width - triangle_len),
    };

    for (int i = 0; i < 2; i++) {
        for (auto it = _frame[i].begin(); it != _frame[i].end(); it++) {
            it->x += viewport->Pos.x;
            it->y += viewport->Pos.y;
        }
    }

    _pos_window = viewport->Pos;
    _sz_window = viewport->Size;

    _eff.init(
        ImVec2(_pos_window.x, _pos_window.y + height_16seg),
        ImVec2(_sz_window.x, _control_height),
        _line_width
    );

    _ttresist.init(
        ImVec2(_pos_window.x, _pos_window.y + height_16seg),
        ImVec2(_sz_window.x, _control_height),
        _line_width
    );

    _led.init(
        ImVec2(_pos_window.x, _pos_window.y + height_16seg),
        ImVec2(_sz_window.x, _control_height),
        _line_width
    );

    // BTNS
    ctx_gui_btn_info tmp;
    int btns = 3;

    auto btn_y = height_16seg + _control_height,
        btn_w = _sz_window.y - btn_y,
        btn_x = (_sz_window.x - (btns * btn_w)) / 2;

    tmp.text = "EFFECTOR";
    tmp.type = E_DISPLAY_TYPE::EFFECTOR;
    tmp.p_box_check[0] = ImVec2(btn_x + 0.f * btn_w, btn_y);
    tmp.p_box_check[1] = ImVec2(btn_x + 1.f * btn_w, _sz_window.y);
    tmp.p_box_draw[0] = ImVec2(
        tmp.p_box_check[0].x + _pos_window.x,
        tmp.p_box_check[0].y + _pos_window.y
    );
    tmp.p_box_draw[1] = ImVec2(
        tmp.p_box_check[1].x + _pos_window.x,
        tmp.p_box_check[1].y + _pos_window.y
    );
    _btn.push_back(tmp);

    tmp.text = "TURNTABLE RESISTANCE";
    tmp.type = E_DISPLAY_TYPE::RESIST;
    tmp.p_box_check[0] = ImVec2(btn_x + 1.f * btn_w, btn_y);
    tmp.p_box_check[1] = ImVec2(btn_x + 2.f * btn_w, _sz_window.y);
    tmp.p_box_draw[0] = ImVec2(
        tmp.p_box_check[0].x + _pos_window.x,
        tmp.p_box_check[0].y + _pos_window.y
    );
    tmp.p_box_draw[1] = ImVec2(
        tmp.p_box_check[1].x + _pos_window.x,
        tmp.p_box_check[1].y + _pos_window.y
    );
    _btn.push_back(tmp);

    tmp.text = "LED CONTROL";
    tmp.type = E_DISPLAY_TYPE::LED_PANEL;
    tmp.p_box_check[0] = ImVec2(btn_x + 2.f * btn_w, btn_y);
    tmp.p_box_check[1] = ImVec2(btn_x + 3.f * btn_w, _sz_window.y);
    tmp.p_box_draw[0] = ImVec2(
        tmp.p_box_check[0].x + _pos_window.x,
        tmp.p_box_check[0].y + _pos_window.y
    );
    tmp.p_box_draw[1] = ImVec2(
        tmp.p_box_check[1].x + _pos_window.x,
        tmp.p_box_check[1].y + _pos_window.y
    );
    _btn.push_back(tmp);

    return 1;
}

static int gui_draw_16seg(void) {
    if (set_coord) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        for (int i = 0; i < 9; i++) {
            _chr[i].draw(drawList);
        }
    }
    return 1;
}

static int gui_draw_control_frame(void) {
    if (set_coord) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        for (int i = 0; i < 2; i++) {
            if (_frame[i].size() >= 3) {
                drawList->AddConvexPolyFilled(
                    _frame[i].data(),
                    static_cast<int>(_frame[i].size()),
                    IM_COL32(255, 255, 255, 255)
                );
            }
        }

        float sz_font = _sz_window.x * 0.03f;
        for (const auto& it : _btn) {
            drawList->AddRectFilled(
                it.p_box_draw[0],
                it.p_box_draw[1],
                _display_type == it.type ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255)
            );

#if 0
            drawList->AddRect(
                it.p_box_draw[0],
                it.p_box_draw[1],
                _display_type == it.type ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255),
                0.f,
                0,
                2.f
            );
#endif

            switch (it.type) {
            case E_DISPLAY_TYPE::EFFECTOR:
                drawList->AddRect(
                    ImVec2(
                        it.p_box_draw[0].x + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f * 0.8f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f * 0.1f
                    ),
                    ImVec2(
                        it.p_box_draw[1].x - (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f * 0.8f,
                        it.p_box_draw[1].y - (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f * 0.1f
                    ),
                    _display_type == it.type ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255),
                    0.f,
                    0,
                    2.f
                );
                drawList->AddRectFilled(
                    ImVec2(
                        it.p_box_draw[0].x + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f * 0.5f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f * 0.7f
                    ),
                    ImVec2(
                        it.p_box_draw[1].x - (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f * 0.5f,
                        it.p_box_draw[1].y - (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f * 0.7f
                    ),
                    _display_type == it.type ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255)
                );
                break;
            case E_DISPLAY_TYPE::RESIST:
                drawList->AddCircle(
                    ImVec2(
                        it.p_box_draw[0].x + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f
                    ),
                    (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.4f,
                    _display_type == it.type ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255),
                    0,
                    2.f
                );

                drawList->AddCircleFilled(
                    ImVec2(
                        it.p_box_draw[0].x + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f
                    ),
                    (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.4f * 0.35f,
                    _display_type == it.type ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255)
                );

                drawList->AddCircleFilled(
                    ImVec2(
                        it.p_box_draw[0].x + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f
                    ),
                    2.f,
                    _display_type == it.type ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255)
                );
                break;
            case E_DISPLAY_TYPE::LED_PANEL:
                drawList->AddCircleFilled(
                    ImVec2(
                        it.p_box_draw[0].x + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f * 0.8f
                    ),
                    (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.25f,
                    _display_type == it.type ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255)
                );

                drawList->AddRectFilled(
                    ImVec2(
                        it.p_box_draw[0].x + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f * 0.75f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f
                    ),
                    ImVec2(
                        it.p_box_draw[1].x - (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.5f * 0.75f,
                        it.p_box_draw[1].y - (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f * 0.25f
                    ),
                    _display_type == it.type ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255),
                    6.f
                );

                drawList->AddRectFilled(
                    ImVec2(
                        it.p_box_draw[0].x + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.1f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f * 0.8f
                        + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.25f
                    ),
                    ImVec2(
                        it.p_box_draw[1].x - (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.1f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.5f * 0.8f
                        + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.325f
                    ),
                    _display_type == it.type ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255)
                );
                break;
            default:
                drawList->AddText(
                    0, sz_font,
                    ImVec2(
                        it.p_box_draw[0].x + (it.p_box_draw[1].x - it.p_box_draw[0].x) * 0.05f,
                        it.p_box_draw[0].y + (it.p_box_draw[1].y - it.p_box_draw[0].y) * 0.25f
                    ),
                    _display_type == it.type ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255),
                    it.text
                );
                break;
            }
            

        }
    }

    return 1;
}

static int gui_draw_effector(void) {
    if (set_coord) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        _eff.draw(drawList);
    }

    return 1;
}

static int gui_draw_tt_resist(void) {
    if (set_coord) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        _ttresist.draw(drawList);
    }

    return 1;
}

static int gui_draw_led(void) {
    if (set_coord) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        _led.draw(drawList);
    }

    return 1;
}

static int get_io(void) {
#ifdef MOUSE_DEBUG
    if (1) {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;
        if (io.MouseDown[0]) {
            _touch_point = mousePos;
            if (ImGui::IsMouseClicked(0)) {
                _touch_state = 1;
            }
            else {
                _touch_state = 2;
            }
        }
        else {
            _touch_state = 0;
        }
    }
#endif

    if (_touch_state == 1) {
        for (const auto& it : _btn) {
            if (_touch_point.x >= it.p_box_check[0].x
                && _touch_point.y >= it.p_box_check[0].y
                && _touch_point.x <= it.p_box_check[1].x
                && _touch_point.y <= it.p_box_check[1].y
                ) {
                _display_type = _display_type == it.type ? E_DISPLAY_TYPE::NONE : it.type;
                break;
            }
        }
    }

    IIDX_SUBMON::E_TOUCH_STATE tstate = IIDX_SUBMON::E_TOUCH_STATE::UP;
    switch (_touch_state) {
    case 1:
        tstate = IIDX_SUBMON::E_TOUCH_STATE::DOWN;
        break;
    case 2:
        tstate = IIDX_SUBMON::E_TOUCH_STATE::HOLD;
        break;
    default:
        tstate = IIDX_SUBMON::E_TOUCH_STATE::UP;
        break;
    }

    switch (_display_type) {
    case E_DISPLAY_TYPE::EFFECTOR:
        _eff.fetch_touch(
            ImVec2(_touch_point.x, _touch_point.y - _16seg_height),
            tstate
        );
        break;
    case E_DISPLAY_TYPE::RESIST:
        _ttresist.fetch_touch(
            ImVec2(_touch_point.x, _touch_point.y - _16seg_height),
            tstate
        );
        break;
    case E_DISPLAY_TYPE::LED_PANEL:
        _led.fetch_touch(
            ImVec2(_touch_point.x, _touch_point.y - _16seg_height),
            tstate
        );
        break;
    }

    for (int i = 0; i < 5; i++) {
        _iodata.slider[i] = _eff.get(i);
    }

    for (int i = 0; i < 2; i++) {
        _iodata.resist[i] = _ttresist.get(i);
    }

    _iodata.led_mask = _led.get();

    for (int i = 0; i < 9; i++) {
        _chr[i].set_draw_ch(_iodata.seg[i]);
    }

    return 1;
}

/*************************************************************************/
bool iidx_sub_gui_set_16seg(unsigned char* data) {
    memcpy(_iodata.seg, data, 9);
    return true;
}

unsigned char iidx_sub_gui_get_resist(char p) {
    return _iodata.resist[p];
}

char iidx_sub_gui_get_slider(char i) {
    if (i >= 0 && i <= 5)
        return _iodata.slider[i] & 0xf;
    else
        return 0;
}

uint32_t iidx_sub_gui_get_led_mask(void) {
    return _iodata.led_mask;
}

/********************************************************************/
void get_touch_info(HWND gui_hwnd, MSG* msg) {
    auto get_pointer = [](HWND gui_hwnd, DWORD pointer_id) {
        POINTER_INFO pointerInfo;
        if (GetPointerInfo(pointer_id, &pointerInfo)) {
            if (pointerInfo.pointerType == PT_TOUCH) {
                POINTER_TOUCH_INFO touchInfo;
                if (GetPointerTouchInfo(pointerInfo.pointerId, &touchInfo)) {
                    // Access touch point data
                    LONG x = pointerInfo.ptPixelLocation.x;
                    LONG y = pointerInfo.ptPixelLocation.y;

                    // Get the source monitor
                    POINT touchPoint = { x, y };
                    HMONITOR sourceMonitor = MonitorFromPoint(touchPoint, MONITOR_DEFAULTTONEAREST);

                    // Get the target monitor (your fullscreen window monitor)
                    HMONITOR targetMonitor = MonitorFromWindow(gui_hwnd, MONITOR_DEFAULTTONEAREST);

                    MONITORINFO srcInfo = { sizeof(MONITORINFO) };
                    MONITORINFO tgtInfo = { sizeof(MONITORINFO) };

                    if (GetMonitorInfo(sourceMonitor, &srcInfo) && GetMonitorInfo(targetMonitor, &tgtInfo)) {
                        // Calculate resolutions of the source and target monitors
                        int srcWidth = srcInfo.rcMonitor.right - srcInfo.rcMonitor.left;
                        int srcHeight = srcInfo.rcMonitor.bottom - srcInfo.rcMonitor.top;

                        int tgtWidth = tgtInfo.rcMonitor.right - tgtInfo.rcMonitor.left;
                        int tgtHeight = tgtInfo.rcMonitor.bottom - tgtInfo.rcMonitor.top;

                        // Map touch coordinates to the target monitor
                        float scaleX = static_cast<float>(tgtWidth) / srcWidth;
                        float scaleY = static_cast<float>(tgtHeight) / srcHeight;

                        int projectedX = static_cast<int>((x - srcInfo.rcMonitor.left) * scaleX);
                        int projectedY = static_cast<int>((y - srcInfo.rcMonitor.top) * scaleY);

                        return ImVec2(static_cast<float>(projectedX), static_cast<float>(projectedY));
                    }
                }
                else {
                    // printf("Failed at GetPointerTouchInfo()\n");
                }
            }
            else {
                // printf("Not PT_TOUCH\n");
            }
        }
        else {
            // printf("Failed at GetPointerInfo()\n");
        }

        return ImVec2(0.f, 0.f);
        };

    switch (msg->message) {
    case WM_POINTERENTER:
        if (_touch_current == 0xffffffff) {
            _touch_current = LOWORD(msg->wParam);
            // printf("ENTER: %d\n", _touch_current);
        }
        break;
    case WM_POINTERLEAVE:
        if (_touch_current == LOWORD(msg->wParam)) {
            // printf("LEAVE: %d\n", _touch_current);
            _touch_current = 0xffffffff;
        }
        break;
    case WM_POINTERDOWN:
        if (_touch_current != 0xffffffff) {
            _touch_state = 1;
            _touch_point = get_pointer(gui_hwnd, _touch_current);

            // printf("DOWN: %d, %f %f\n", _touch_current, _touch_point.x, _touch_point.y);
        }
        break;
    case WM_POINTERUP:
        if (_touch_current == LOWORD(msg->wParam)) {
            _touch_state = 0;
            _touch_point = ImVec2(0, 0);

            // printf("UP: %d, %f %f\n", _touch_current, _touch_point.x, _touch_point.y);
        }
        break;
    case WM_POINTERUPDATE:
        if (_touch_current == LOWORD(msg->wParam)) {
            _touch_state = 2;
            _touch_point = get_pointer(gui_hwnd, _touch_current);

            // printf("MOVE: %d, %f %f\n", _touch_current, _touch_point.x, _touch_point.y);
        }
        break;
    }
}
