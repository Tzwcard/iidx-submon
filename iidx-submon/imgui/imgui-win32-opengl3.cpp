// Dear ImGui: standalone example application for Win32 + OpenGL 3

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// This is provided for completeness, however it is strongly recommended you use OpenGL with SDL or GLFW.

#include "imgui.h"
#include "backend/imgui_impl_opengl3.h"
#include "backend/imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>
#include <iostream>

#include "imgui-win32-opengl3.h"
#include "../misc/flexure.ttf.h"

#include "iidx-sub-gui.h"

#pragma comment(lib, "opengl32.lib")

#ifdef DEBUG_BUILD
#define USE_CUSTOM_RESOLUTION
// #define DRAW_IMG_CLICKER
// #define ENABLE_SVG_EXPORT
#endif

#ifdef DRAW_IMG_CLICKER
#include "imgtest.h"
#endif

#ifdef ENABLE_SVG_EXPORT
#include "drawsvg.h"
#endif

#define SUBMON_WIDTH  1920
#define SUBMON_HEIGHT 1080

struct CTX_FINDMONITOR {
    HMONITOR gameMonitor;
    HMONITOR targetMonitor;
};

// Data stored per platform window
struct WGL_WindowData { HDC hDC; };

// Data
static HGLRC            g_hRC;
static WGL_WindowData   g_MainWindow;
static int              g_Width;
static int              g_Height;

static HWND hwnd_submon = nullptr, hwnd_iidx = nullptr;
static HMONITOR hmon_submon = nullptr;
static int iidx_slider_type = -1;

// Forward declarations of helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
void ResetDeviceWGL();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"CLS_BM2DX_BI2X_SUBMONITOR", nullptr };

static void SetMonitorResolution(MONITORINFOEX*);
static BOOL CALLBACK WindowEnumProcFind(HWND hwnd, LPARAM lParam);
static BOOL CALLBACK MonitorEnumProcCount(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
static BOOL CALLBACK MonitorEnumProcGet(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

static int gui_get_monitor_nr(void);
static int gui_get_submon_and_game_hwnd(void);

static void InstallTouchHook();
static void UninstallTouchHook();

static bool done = false;

// Support function for multi-viewports
// Unlike most other backend combination, we need specific hooks to combine Win32+OpenGL.
// We could in theory decide to support Win32-specific code in OpenGL backend via e.g. an hypothetical ImGui_ImplOpenGL3_InitForRawWin32().
static void Hook_Renderer_CreateWindow(ImGuiViewport* viewport)
{
    assert(viewport->RendererUserData == NULL);

    WGL_WindowData* data = IM_NEW(WGL_WindowData);
    CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
    viewport->RendererUserData = data;
}

static void Hook_Renderer_DestroyWindow(ImGuiViewport* viewport)
{
    if (viewport->RendererUserData != NULL)
    {
        WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData;
        CleanupDeviceWGL((HWND)viewport->PlatformHandle, data);
        IM_DELETE(data);
        viewport->RendererUserData = NULL;
    }
}

static void Hook_Platform_RenderWindow(ImGuiViewport* viewport, void*)
{
    // Activate the platform window DC in the OpenGL rendering context
    if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
        wglMakeCurrent(data->hDC, g_hRC);
}

static void Hook_Renderer_SwapBuffers(ImGuiViewport* viewport, void*)
{
    if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
        ::SwapBuffers(data->hDC);
}

// Main code
void gui_close(void) {
    done = true;
}

int gui_init(void) {
    return gui_get_monitor_nr() >= 2 ? 1 : -1;
}

int gui_main(void)
{
    hmon_submon = nullptr;
    hwnd_iidx = nullptr;

    gui_get_submon_and_game_hwnd();

    if (!hmon_submon) {
        printf("Cannot identify sub monitor!\n");
        return -101;
    }

    MONITORINFOEX mi;
    mi.cbSize = sizeof(mi);

    if (!GetMonitorInfo(hmon_submon, &mi)) {
        printf("Cannot get monitor info!\n");
        return -102;
    }

    printf("About to start gui_main()...\n");

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    ::RegisterClassExW(&wc);
    hwnd_submon = ::CreateWindowExW(0,
        wc.lpszClassName,
        L"BM2DX_SUB_WINDOW",
        WS_POPUP,
#ifdef USE_CUSTOM_RESOLUTION
        mi.rcMonitor.left, mi.rcMonitor.top,
        SUBMON_WIDTH, SUBMON_HEIGHT,
#else
        mi.rcMonitor.left, mi.rcMonitor.top,
        mi.rcMonitor.right - mi.rcMonitor.left,
        mi.rcMonitor.bottom - mi.rcMonitor.top,
#endif
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    if (!hwnd_submon) {
        printf("Failed to CreateWindowExW()\n");
        return 1;
    }

    // Initialize OpenGL
    if (!CreateDeviceWGL(hwnd_submon, &g_MainWindow))
    {
        printf("Failed to CreateDeviceWGL()\n");

        CleanupDeviceWGL(hwnd_submon, &g_MainWindow);
        ::DestroyWindow(hwnd_submon);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    // Show the window
    ::ShowWindow(hwnd_submon, SW_SHOWNOACTIVATE);
    ::UpdateWindow(hwnd_submon);

    bool is_touch_available = true;
    if (RegisterTouchWindow(hwnd_submon, 0) == FALSE) {
        printf("Failed to register touch window.\n");
        is_touch_available = false;
    }
    else {
        if (hwnd_iidx) {
            RegisterTouchWindow(hwnd_iidx, 0);
            InstallTouchHook();
        }
    }

    printf("Window created.\n");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows
    io.IniFilename = NULL;
    // io.FontGlobalScale = 1.f;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(hwnd_submon);
    ImGui_ImplOpenGL3_Init();

    // Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32 and GL api.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        IM_ASSERT(platform_io.Renderer_CreateWindow == NULL);
        IM_ASSERT(platform_io.Renderer_DestroyWindow == NULL);
        IM_ASSERT(platform_io.Renderer_SwapBuffers == NULL);
        IM_ASSERT(platform_io.Platform_RenderWindow == NULL);
        platform_io.Renderer_CreateWindow = Hook_Renderer_CreateWindow;
        platform_io.Renderer_DestroyWindow = Hook_Renderer_DestroyWindow;
        platform_io.Renderer_SwapBuffers = Hook_Renderer_SwapBuffers;
        platform_io.Platform_RenderWindow = Hook_Platform_RenderWindow;
    }

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // According to github, ImGui will take ownership of the memory
    // that malloc() and free() it itself
    void* font_data_load = NULL;
    ImFont* flexure = NULL;
    {
        const unsigned char* font_data = NULL;
        long font_size = _get_font(&font_data);

        if (font_size > 0) {
            font_data_load = malloc(font_size);
            memcpy(font_data_load, font_data, font_size);
            // io.Fonts->Clear();
            io.Fonts->AddFontDefault();
            flexure = io.Fonts->AddFontFromMemoryTTF(font_data_load, font_size, 60.f, NULL, io.Fonts->GetGlyphRangesDefault());
            iidx_sub_gui_set_panel_font(flexure);
            io.Fonts->Build();
        }
    }

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Set slider text type here
    iidx_sub_gui_set_silder_type(iidx_slider_type);

    // Main loop
    done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        iidx_sub_gui();
#ifdef DRAW_IMG_CLICKER
        RenderImageWithClick();
#endif
#if 0
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
#endif
        // Rendering
        ImGui::Render();

#ifdef ENABLE_SVG_EXPORT
        CheckSVGExport();
#endif

        glViewport(0, 0, g_Width, g_Height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();

            // Restore the OpenGL rendering context to the main window DC, since platform windows might have changed it.
            wglMakeCurrent(g_MainWindow.hDC, g_hRC);
        }

        // Present
        ::SwapBuffers(g_MainWindow.hDC);
    }

    iidx_sub_gui_set_panel_font(NULL);

    if (is_touch_available) {
        if (hwnd_iidx) {
            UninstallTouchHook();
            UnregisterTouchWindow(hwnd_iidx);
        }
        UnregisterTouchWindow(hwnd_submon);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceWGL(hwnd_submon, &g_MainWindow);
    wglDeleteContext(g_hRC);
    ::DestroyWindow(hwnd_submon);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    HDC hDc = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0)
        return false;
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
        return false;
    ::ReleaseDC(hWnd, hDc);

    data->hDC = ::GetDC(hWnd);
    if (!g_hRC)
        g_hRC = wglCreateContext(data->hDC);
    return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    wglMakeCurrent(nullptr, nullptr);
    ::ReleaseDC(hWnd, data->hDC);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            g_Width = LOWORD(lParam);
            g_Height = HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

/*********************************** My Functions ***********************************/
static BOOL CALLBACK MonitorEnumProcCount(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    auto c = reinterpret_cast<int*>(dwData);
    *c += 1;
    return TRUE;
}

static BOOL CALLBACK MonitorEnumProcGet(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    auto target = reinterpret_cast<CTX_FINDMONITOR*>(dwData);
    if (hMonitor != target->gameMonitor) {
        target->targetMonitor = hMonitor;
        return FALSE;
    }

    return TRUE;
}

static const char* bm2dx_class_prefix[] = {
    "beatmaniaIIDX",
    "beatmania IIDX",
};

static BOOL CALLBACK WindowEnumProcFind(HWND hwnd, LPARAM lParam) {
    auto data = reinterpret_cast<HWND*>(lParam);
    char className[256] = { 0 };

    if (GetClassNameA(hwnd, className, sizeof(className))) {
        bool is_match = false;
        if (!strcmp("C02", className)) {
            is_match = true;
            iidx_slider_type = 1;
        }
        else {
            for (int i = 0; i < _countof(bm2dx_class_prefix) && !is_match; i++) {
                if (!strncmp(bm2dx_class_prefix[i], className, strlen(bm2dx_class_prefix[i]))) {
                    is_match = true;
                    iidx_slider_type = 1;
                }
            }
        }

        // Add MAME emulator check if want to use on Twinkle games
        // And because only old styles(1st - 8th) uses "track volume" instead of "filter" effector
        // that conviently only runs under MAME(by far) we can set the silder type here as 0
        if (!is_match && !strcmp("MAME", className)) {
            is_match = true;
            iidx_slider_type = 0;
        }

        if (is_match) {
            printf("Found game window '%s'\n", className);
            *data = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

static void SetMonitorResolution(MONITORINFOEX* mi) {
    DEVMODE dm = {};
    dm.dmSize = sizeof(DEVMODE);
    dm.dmPelsWidth = SUBMON_WIDTH;  // Desired width
    dm.dmPelsHeight = SUBMON_HEIGHT;  // Desired height
    dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

    // Change display settings
    if (ChangeDisplaySettingsEx(mi->szDevice, &dm, nullptr, CDS_FULLSCREEN, nullptr) != DISP_CHANGE_SUCCESSFUL) {
        printf("Failed to change displaysettings: %S\n", mi->szDevice);
    }
}

static int gui_get_submon_and_game_hwnd(void) {
#ifndef DEBUG_BUILD
    // Find game window
    int init_retry = 10;
    HWND game_window = NULL;

    while (init_retry-- > 0) {
        game_window = NULL;
        EnumWindows(WindowEnumProcFind, reinterpret_cast<LPARAM>(&game_window));
        if (game_window) {
            break;
        }
        Sleep(1000);
    }

    if (!game_window) {
        printf("Cannot find game window after 10 seconds...\n");
        return -100;
    }

    hwnd_iidx = game_window;

    // Make sure game window is opened and full screened
    printf("Wait...\n");
    Sleep(5000);

    RECT rect;
    if (GetWindowRect(game_window, &rect)) {
        HMONITOR hmon = MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);
        if (hmon) {
            CTX_FINDMONITOR find;
            find.targetMonitor = NULL;
            find.gameMonitor = hmon;

            EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProcGet, reinterpret_cast<LPARAM>(&find));

            hmon_submon = find.targetMonitor;
        }
    }
#else
    CTX_FINDMONITOR find;
    find.targetMonitor = NULL;
    find.gameMonitor = NULL;

    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProcGet, reinterpret_cast<LPARAM>(&find));

    hmon_submon = find.targetMonitor;
#endif

    return 1;
}

static int gui_get_monitor_nr(void) {
#ifndef DEBUG_BUILD
    int nr_monitor = 0;
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProcCount, reinterpret_cast<LPARAM>(&nr_monitor));

    if (nr_monitor < 2) {
        printf("Cannot find more than one monitor!\n");
        return -1;
    }
    printf("Total %d monitors\n", nr_monitor);

    return nr_monitor;
#else
    printf("********** IO DEBUG BUILD **********\n");
    printf(DEBUG_BUILD_WARNING_INFO);
    return 2;
#endif
}

/********************** REMOTE TOUCH HOOK **********************/
static HHOOK hHook;

static LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code >= 0 && wParam == PM_REMOVE) {
        MSG* msg = reinterpret_cast<MSG*>(lParam);

        if (msg->message == WM_POINTERDOWN
            || msg->message == WM_POINTERUPDATE
            || msg->message == WM_POINTERENTER
            || msg->message == WM_POINTERUP
            || msg->message == WM_POINTERLEAVE) {
            get_touch_info(hwnd_submon, msg);
        }
    }
    return CallNextHookEx(hHook, code, wParam, lParam);
}

static void InstallTouchHook() {
    if (!hwnd_iidx) return;

    DWORD processId;
    DWORD threadId = GetWindowThreadProcessId(hwnd_iidx, &processId);
    if (threadId == 0) {
        printf("Cannot get thread id from game window!\n");
        return;
    }

    hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, nullptr, threadId);
}

static void UninstallTouchHook() {
    if (hHook) UnhookWindowsHookEx(hHook);
}