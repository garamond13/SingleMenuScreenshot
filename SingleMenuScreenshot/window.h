#pragma once

#include "pch.h"
#include "resource.h"

enum SMSS_HOTKEY_
{
	SMSS_HOTKEY_PRINTSCREEN,
	SMSS_HOTKEY_ALT_PRINTSCREEN,
	SMSS_HOTKEY_CTRL_PRINTSCREEN
};

class Window
{
public:
	Window(HINSTANCE hinstance);
	int message_loop();
private:
	static LRESULT CALLBACK wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	void create_tray_icon();
	void register_hotkeys() const noexcept;
	void show_menu(const POINT& point) const noexcept;
	void set_check_menu_items(HMENU hmenu) const noexcept;
	HWND hwnd;
};
