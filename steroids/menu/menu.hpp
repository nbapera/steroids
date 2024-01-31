#pragma once
#include <windows.h>
#include "../imgui/imgui.h"

namespace menu {
	void run();
	void set_position(int x, int y, int cx, int cy, HWND hwnd);
	void get_mouse_offset(int& x, int& y, HWND hwnd);

	inline bool open = true;

	inline int menu_width = { 730 };
	inline int menu_height = { 460 };

	inline int tab = 0;
	inline int subtab = 0;

	inline float main_color[3] = { 83.f / 255, 127.f / 255, 252.f / 255 };

	void rainbow_menu();
	void keybind_button(int& i_key, int i_width, int i_height);

	inline int menu_key = VK_INSERT;
	inline int destruct = VK_DELETE;


	inline bool epilepsy_mode = false;
	inline bool rainbow = false;

	void listen_keybinds();

}
