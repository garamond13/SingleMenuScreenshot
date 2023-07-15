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
	void create(HINSTANCE hinstance);
private:
	static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	void create_tray_icon();
	void register_hotkeys() const noexcept;
	void wm_hotkey(WPARAM wparam) const;
	void wm_command(WPARAM wparam) const;
	void show_menu(const POINT& point) const noexcept;
	void set_check_menu_items(HMENU hmenu) const noexcept;
	void destroy_tray_icon() const noexcept;
	HWND hwnd;
};
