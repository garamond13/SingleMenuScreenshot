module;
#include "framework.h"
#include "resource.h"

export module window;
import config;
import screenshot;

namespace {
	constexpr auto WM_APP_NOTIFYCALLBACK{ WM_APP + 1 };
	constexpr wchar_t CLASS_NAME[]{ L"SMSS" };
}

export class Window {
public:
	void initialize(HINSTANCE hinstance)
	{
		//register window class
		WNDCLASSEXW wndclassexw{
			.cbSize{ sizeof(WNDCLASSEXW) },
			.lpfnWndProc{ window_procedure },
			.hInstance{ hinstance },
			.hIcon{ LoadIconW(hinstance, MAKEINTRESOURCEW(IDI_ICON)) },
			.lpszClassName{ CLASS_NAME },
		};
		RegisterClassExW(&wndclassexw);

		//create message only window
		hwnd = CreateWindowExW(0, CLASS_NAME, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hinstance, this);
		if (!hwnd)
			throw;

		create_tray_icon(hinstance);
		register_hotkeys();

		//if the config doesnt exist it will create it
		config.read();
	}

private:
	//hotkeys have to be set with RegisterHotKey()
	void register_hotkeys()
	{
		RegisterHotKey(hwnd, 1, MOD_NOREPEAT, VK_SNAPSHOT);
		RegisterHotKey(hwnd, 2, MOD_ALT | MOD_NOREPEAT, VK_SNAPSHOT);
		RegisterHotKey(hwnd, 3, MOD_CONTROL | MOD_NOREPEAT, VK_SNAPSHOT);
	}

	void create_tray_icon(HINSTANCE hinstance)
	{
		NOTIFYICONDATAW notifyicondata{
			.cbSize{ sizeof(NOTIFYICONDATAW) },
			.hWnd{ hwnd },
			.uID{ IDI_ICON },
			.uFlags{ NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP },
			.uCallbackMessage{ WM_APP_NOTIFYCALLBACK },
			.hIcon{ LoadIconW(hinstance, MAKEINTRESOURCEW(IDI_ICON)) },
			.uVersion{ NOTIFYICON_VERSION_4 },
		};
		//initialize .szTip
		//version numbering major.minor.patch
		constexpr wchar_t tip[]{ L"Single Menu Screenshot 1.0.0" };
		wcscpy_s(notifyicondata.szTip, sizeof(tip), tip);

		Shell_NotifyIconW(NIM_ADD, &notifyicondata);
		Shell_NotifyIconW(NIM_SETVERSION, &notifyicondata);
	}

	//this is the main window procedure
	static LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		//equivalent to the this pointer of the Window class
		static Window* window;
		switch (message) {

			//initialize window and hwnd as soon as possible
		case WM_NCCREATE:
			window = reinterpret_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);
			window->hwnd = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
			return DefWindowProcW(hwnd, message, wparam, lparam);

		case WM_HOTKEY:
			return window->wm_hotkey(lparam);
		case WM_APP_NOTIFYCALLBACK:
			return window->wm_app_notifycallback(message, wparam, lparam);
		case WM_COMMAND:
			window->wm_command(wparam);
			break;
		case WM_DESTROY:
			window->destroy_tray_icon();
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProcW(hwnd, message, wparam, lparam);
		}
		return 0;
	}

	LRESULT wm_app_notifycallback(UINT message, WPARAM wparam, LPARAM lparam)
	{
		if (LOWORD(lparam) == WM_CONTEXTMENU) {
			POINT point{ LOWORD(wparam), HIWORD(wparam) };
			show_menu(point);
			return 0;
		}
		return DefWindowProcW(hwnd, message, wparam, lparam);
	}

	LRESULT wm_hotkey(LPARAM lparam)
	{
		switch (LOWORD(lparam)) {
		case MOD_ALT: {
			Screenshot screenshot(&config);
			screenshot.client();
		}
					break;
		case MOD_CONTROL: {
			Screenshot screenshot(&config);
			screenshot.window();
		}
						break;
		default: {
			Screenshot screenshot(&config);
			screenshot.fullscreen();
		}
		}
		return 0;
	}

	void wm_command(WPARAM wparam)
	{
		switch (LOWORD(wparam)) {
		case ID_AUTOSTART:
			config.set_autostart();
			return;
		case ID_FOLDER:
			config.get_folder();
			return;
		case ID_FORMAT_PNG:
			if (config.flags & Config::FORMAT_BMP)
				config.flags ^= Config::FORMAT_BMP;
			config.flags |= Config::FORMAT_PNG;
			config.write();
			return;
		case ID_FORMAT_BMP:
			if (config.flags & Config::FORMAT_PNG)
				config.flags ^= Config::FORMAT_PNG;
			config.flags |= Config::FORMAT_BMP;
			config.write();
			return;
		case ID_EXIT:
			DestroyWindow(hwnd);
		}
	}

	void destroy_tray_icon()
	{
		NOTIFYICONDATAW notifyicondata{
			.cbSize{ sizeof(NOTIFYICONDATAW) },
			.hWnd{ hwnd },
			.uID{ IDI_ICON },
		};
		Shell_NotifyIconW(NIM_DELETE, &notifyicondata);
	}

	void show_menu(const POINT& point)
	{
		auto hmenu{ LoadMenuW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDR_MENU)) };
		auto sub_menu{ GetSubMenu(hmenu, 0) };
		set_check_menu_items(sub_menu);

		//window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
		SetForegroundWindow(hwnd);

		//respect menu drop alignment
		auto flags{ TPM_RIGHTBUTTON };
		if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
			flags |= TPM_RIGHTALIGN;
		else
			flags |= TPM_LEFTALIGN;

		TrackPopupMenuEx(sub_menu, flags, point.x, point.y, hwnd, nullptr);
		DestroyMenu(hmenu);
	}

	//sets check marks on menu items 
	void set_check_menu_items(HMENU hmenu)
	{
		if (config.flags & Config::FORMAT_PNG)
			CheckMenuItem(hmenu, ID_FORMAT_PNG, MF_CHECKED);
		if (config.flags & Config::FORMAT_BMP)
			CheckMenuItem(hmenu, ID_FORMAT_BMP, MF_CHECKED);
		if (config.flags & Config::AUTOSTART)
			CheckMenuItem(hmenu, ID_AUTOSTART, MF_CHECKED);
	}

	HWND hwnd;
	Config config;
};
