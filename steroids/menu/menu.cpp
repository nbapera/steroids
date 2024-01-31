#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include "byte_array.hpp"
#include <d3d9.h>
#include <tchar.h>
#include <windows.h>
#include <unordered_map>
#include <cmath>
#include <string>
#include <thread>
#include <chrono>

#include "nav.hpp"
#include "etc.hpp"
#include "../features/clicker.hpp"
#include "keycodes.hpp"

#pragma comment (lib, "d3d9.lib")
#include "menu.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ImFont* medium;
ImFont* bold;
ImFont* tab_icons;
ImFont* logo;
ImFont* tab_title;
ImFont* tab_title_icon;
ImFont* subtab_title;
ImFont* combo_arrow;

static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

inline HWND hwnd;

enum heads {
    rage, antiaim, visuals, settings, skins, configs, scripts
};
enum sub_heads {
    general, _general,
};

void menu::listen_keybinds() {
    while (true) {

        if (GetAsyncKeyState(menu::menu_key)) {
            open = !open;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (GetAsyncKeyState(aclicker::left_clicker_key)) {
            aclicker::toggled = !aclicker::toggled;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (GetAsyncKeyState(menu::destruct)) {
            exit(0);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

std::string get_key_name_by_id(int id)
{
    static std::unordered_map<int, std::string> key_names = {
        { 0, "None" },
        { VK_LBUTTON, "Mouse 1" },
        { VK_RBUTTON, "Mouse 2" },
        { VK_MBUTTON, "Mouse 3" },
        { VK_XBUTTON1, "Mouse 4" },
        { VK_XBUTTON2, "Mouse 5" },
        { VK_BACK, "Back" },
        { VK_TAB, "Tab" },
        { VK_CLEAR, "Clear" },
        { VK_RETURN, "Enter" },
        { VK_SHIFT, "Shift" },
        { VK_CONTROL, "Ctrl" },
        { VK_MENU, "Alt" },
        { VK_PAUSE, "Pause" },
        { VK_CAPITAL, "Caps Lock" },
        { VK_ESCAPE, "Escape" },
        { VK_SPACE, "Space" },
        { VK_PRIOR, "Page Up" },
        { VK_NEXT, "Page Down" },
        { VK_END, "End" },
        { VK_HOME, "Home" },
        { VK_LEFT, "Left Key" },
        { VK_UP, "Up Key" },
        { VK_RIGHT, "Right Key" },
        { VK_DOWN, "Down Key" },
        { VK_SELECT, "Select" },
        { VK_PRINT, "Print Screen" },
        { VK_INSERT, "Insert" },
        { VK_DELETE, "Delete" },
        { VK_HELP, "Help" },
        { VK_SLEEP, "Sleep" },
        { VK_MULTIPLY, "*" },
        { VK_ADD, "+" },
        { VK_SUBTRACT, "-" },
        { VK_DECIMAL, "." },
        { VK_DIVIDE, "/" },
        { VK_NUMLOCK, "Num Lock" },
        { VK_SCROLL, "Scroll" },
        { VK_LSHIFT, "Left Shift" },
        { VK_RSHIFT, "Right Shift" },
        { VK_LCONTROL, "Left Ctrl" },
        { VK_RCONTROL, "Right Ctrl" },
        { VK_LMENU, "Left Alt" },
        { VK_RMENU, "Right Alt" },
    };

    if (id >= 0x30 && id <= 0x5A)
        return std::string(1, (char)id);

    if (id >= 0x60 && id <= 0x69)
        return "Num " + std::to_string(id - 0x60);

    if (id >= 0x70 && id <= 0x87)
        return "F" + std::to_string((id - 0x70) + 1);

    return key_names[id];
}

void menu::keybind_button(int& i_key, int i_width, int i_height)
{
    static auto b_get = false;
    static std::string sz_text("Click to bind");

    if (ImGui::Button(sz_text.c_str(), ImVec2((float)(i_width), (float)(i_height))))
        b_get = true;

    if (b_get)
    {
        for (auto i = 1; i < 256; i++)
        {
            if (GetAsyncKeyState(i) & 0x8000)
            {
                if (i != 12)
                {
                    i_key = i == VK_ESCAPE ? 0 : i;
                    b_get = false;
                }
            }
        }
        sz_text = "Press a key";
    }
    else if (!b_get && i_key == 0)
        sz_text = "Click to bind";
    else if (!b_get && i_key != 0)
        sz_text = "Bound to " + get_key_name_by_id(i_key);
}



void menu::set_position(int x, int y, int cx, int cy, HWND hwnd) {
    POINT point; GetCursorPos(&point);

    auto flags = SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE;
    if (x != 0 && y != 0)
    {
        x = point.x - x;
        y = point.y - y;
        flags &= ~SWP_NOMOVE;
    }

    if (cx != 0 && cy != 0)
        flags &= ~SWP_NOSIZE;

    SetWindowPos(hwnd, nullptr, x, y, cx, cy, flags);
}

void menu::get_mouse_offset(int& x, int& y, HWND hwnd)
{
    POINT point; RECT rect;

    GetCursorPos(&point);
    GetWindowRect(hwnd, &rect);

    x = point.x - rect.left;
    y = point.y - rect.top;
}

// Main code
void menu::run()
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle( NULL ), NULL, NULL, NULL, NULL, L"Class", NULL };
    ::RegisterClassEx(&wc);
    hwnd = CreateWindow(wc.lpszClassName, L"", WS_POPUP, 0, 0, menu::menu_width, menu::menu_height, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    ImFontConfig font_config;
    font_config.PixelSnapH = false;
    font_config.OversampleH = 5;
    font_config.OversampleV = 5;
    font_config.RasterizerMultiply = 1.2f;

    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0xE000, 0xE226, // icons
        0,
    };

    font_config.GlyphRanges = ranges;


    medium = io.Fonts->AddFontFromMemoryTTF(PTRootUIMedium, sizeof(PTRootUIMedium), 15.0f, &font_config, ranges);
    bold = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold, sizeof(PTRootUIBold), 15.0f, &font_config, ranges);

    tab_icons = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 15.0f, &font_config, ranges);
    logo = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 21.0f, &font_config, ranges);

    tab_title = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold, sizeof(PTRootUIBold), 19.0f, &font_config, ranges);
    tab_title_icon = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 18.0f, &font_config, ranges);

    subtab_title = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold, sizeof(PTRootUIBold), 15.0f, &font_config, ranges);

    combo_arrow = io.Fonts->AddFontFromMemoryTTF(combo, sizeof(combo), 9.0f, &font_config, ranges);





    // Our state
    ImVec4 clear_color = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();


        const char* tab_name = tab == rage ? "Autoclicker" : tab == antiaim ? "Anti-aim" : tab == visuals ? "Visuals" : tab == settings ? "Settings" : tab == skins ? "Skins" : tab == configs ? "Configs" : tab == scripts ? "Scripts" : 0;
        const char* tab_icon = tab == rage ? "B" : tab == antiaim ? "C" : tab == visuals ? "D" : tab == settings ? "E" : tab == skins ? "F" : tab == configs ? "G" : tab == scripts ? "H" : 0;

        static bool boolean, boolean_1 = false;
        static int sliderscalar, combo, combo_right = 0;

        const char* combo_items[3] = { "Jitter", "Butterfly", "Normal" };
        const char* combo_items_right[3] = { "Jitter", "Butterfly", "Normal" };

        !open ? ShowWindow(hwnd, SW_HIDE) : ShowWindow(hwnd, SW_SHOW);

        ImGui::SetNextWindowSize({ (float)menu::menu_width, (float)menu::menu_height }, ImGuiCond_Once);
        ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Once);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(main_color[0], main_color[1], main_color[2], 1.f));

        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(main_color[0], main_color[1], main_color[2], 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(main_color[0], main_color[1], main_color[2], 1.f));

        static int x = 0, y = 0;

        ImGui::Begin("hi world", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove); {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                get_mouse_offset(x, y, hwnd);

            if (y >= 0 && y <= (ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 4) && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                set_position(x, y, menu::menu_width, menu::menu_height, hwnd);

            auto draw = ImGui::GetWindowDrawList();

            auto pos = ImGui::GetWindowPos();
            auto size = ImGui::GetWindowSize();

            ImGuiStyle style = ImGui::GetStyle();

            draw->AddRectFilled(pos, ImVec2(pos.x + 210, pos.y + size.y), ImColor(24, 24, 26), style.WindowRounding, ImDrawFlags_RoundCornersLeft);
            draw->AddLine(ImVec2(pos.x + 210, pos.y + 2), ImVec2(pos.x + 210, pos.y + size.y - 2), ImColor(1.0f, 1.0f, 1.0f, 0.03f));
            draw->AddLine(ImVec2(pos.x + 47, pos.y + 2), ImVec2(pos.x + 47, pos.y + size.y - 2), ImColor(1.0f, 1.0f, 1.0f, 0.03f));
            draw->AddLine(ImVec2(pos.x + 2, pos.y + 47), ImVec2(pos.x + 47, pos.y + 47), ImColor(1.0f, 1.0f, 1.0f, 0.03f));
            draw->AddLine(ImVec2(pos.x + 63, pos.y + 47), ImVec2(pos.x + 195, pos.y + 47), ImColor(1.0f, 1.0f, 1.0f, 0.03f));
            draw->AddText(logo, 21.0f, ImVec2(pos.x + 14, pos.y + 12), ImColor(main_color[0], main_color[1], main_color[2]), "A");

            draw->AddText(tab_title_icon, 18.0f, ImVec2(pos.x + 65, pos.y + 14), ImColor(main_color[0], main_color[1], main_color[2]), tab_icon);
            draw->AddText(tab_title, 19.0f, ImVec2(pos.x + 93, pos.y + 15), ImColor(1.0f, 1.0f, 1.0f), tab_name);

            draw->AddRect(pos + ImVec2(1, 1), pos + size - ImVec2(1, 1), ImColor(1.0f, 1.0f, 1.0f, 0.03f), style.WindowRounding);

            ImGui::SetCursorPos({ 8, 56 });
            ImGui::BeginGroup(); {
                if (elements::tab("B", tab == rage)) { tab = rage; }
                if (elements::tab("E", tab == settings)) { tab = settings; }
            } ImGui::EndGroup();

            switch (tab) {
            case rage:
                subtab = general;
                draw->AddText(subtab_title, 15.0f, ImVec2(pos.x + 72, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.4f), "MAIN");

                ImGui::SetCursorPos({ 57, 86 });
                ImGui::BeginGroup(); {
                    if (elements::subtab("General", subtab == general)) { subtab = general; }
                } ImGui::EndGroup();

                switch (subtab) {
                case general:
                    ImGui::SetCursorPos({ 226, 16 });
                    e_elements::begin_child("Left Clicker", ImVec2(240, 430)); {
                        ImGui::Checkbox("Enabled", &aclicker::toggled, main_color[0], main_color[1], main_color[2]);
                        ImGui::Checkbox("Randomized", &aclicker::randomized, main_color[0], main_color[1], main_color[2]);
                        ImGui::Checkbox("Blockhit", &boolean_1, main_color[0], main_color[1], main_color[2]);

                        ImGui::SliderInt("CPS", &aclicker::cps, 0, 20, "%d%CPS", ImGuiSliderFlags_None, main_color[0], main_color[1], main_color[2]);

                        ImGui::Combo("Clicking Type", &combo, combo_items, IM_ARRAYSIZE(combo_items));
                        
                        ImGui::Text("Clicker keybind");
                        keybind_button(aclicker::left_clicker_key, 200, 30);


                    }
                    e_elements::end_child();

                    ImGui::SetCursorPos({ 476, 16 });
                    e_elements::begin_child("Right Clicker", ImVec2(240, 430)); {
                        ImGui::Checkbox("Enabled", &aclicker::toggled_right, main_color[0], main_color[1], main_color[2]);
                        ImGui::Checkbox("Randomized", &aclicker::randomized_right, main_color[0], main_color[1], main_color[2]);

                        ImGui::SliderInt("CPS", &aclicker::cps_right, 0, 25, "%d%CPS", ImGuiSliderFlags_None, main_color[0], main_color[1], main_color[2]);

                        ImGui::Combo("Clicking Type", &combo_right, combo_items_right, IM_ARRAYSIZE(combo_items_right));

                    }
                    e_elements::end_child();

                    break;
                }
                break;
            case settings:
                subtab = _general;
                draw->AddText(subtab_title, 15.0f, ImVec2(pos.x + 72, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.4f), "MAIN");

                ImGui::SetCursorPos({ 57, 86 });
                ImGui::BeginGroup(); {
                    if (elements::subtab("General", subtab == _general)) { subtab = _general; }
                } ImGui::EndGroup();
                switch (subtab) {
                case _general:
                    ImGui::SetCursorPos({ 226, 16 });
                    e_elements::begin_child("Settings", ImVec2(240, 430)); {
                        ImGui::ColorPicker4("Main Color", menu::main_color);
                        ImGui::Checkbox("Rainbow Mode", &rainbow, main_color[0], main_color[1], main_color[2]);

                        if (ImGui::Button("Self-Destruct", ImVec2(200, 30))) {
                            exit(0);
                        }

                    }
                    e_elements::end_child();

                    ImGui::SetCursorPos({ 476, 16 });
                    e_elements::begin_child("Keybinds", ImVec2(240, 430)); {
                        ImGui::Text("Open Menu Keybind");
                        keybind_button(menu_key, 201, 30);

                    }
                    e_elements::end_child();

                    break;
                }
            }

        }
        ImGui::End();

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return;
}

void menu::rainbow_menu() {

    const float duration = 3.0f;
    const int steps = 100;

    while (true) {

        if (!rainbow) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }

        for (int i = 0; i <= steps; ++i) {
            float percentage = static_cast<float>(i) / static_cast<float>(steps);

            main_color[0] = 0.5f + 0.5f * std::sin(2 * M_PI * percentage);
            main_color[1] = 0.5f + 0.5f * std::sin(2 * M_PI * (percentage + 1.0f / 3.0f));
            main_color[2] = 0.5f + 0.5f * std::sin(2 * M_PI * (percentage + 2.0f / 3.0f));

            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(duration * 1000 / steps)));
        }
    }

}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
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
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}