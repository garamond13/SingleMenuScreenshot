#include "pch.h"
#include "window.h"
#include "global.h"
#include "screenshot.h"
#include "helpers.h"
#include "version.h"

constexpr auto SMSS_WM_APP_NOTIFYCALLBACK{ WM_USER + 0 };

void Window::create(HINSTANCE hinstance)
{
	const WNDCLASSEXW wndclassexw{
		.cbSize{ sizeof(WNDCLASSEXW) },
		.lpfnWndProc{ wndproc },
		.hInstance{ hinstance },
		.hIcon{ LoadIconW(hinstance, MAKEINTRESOURCEW(IDI_ICON)) },
		.lpszClassName{ L"smss" },
	};
	smss_assert(RegisterClassExW(&wndclassexw), != 0);

	//create message only window
	smss_assert(CreateWindowExW(0, wndclassexw.lpszClassName, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hinstance, this), != nullptr);

	create_tray_icon();
	register_hotkeys();
}

/* static */ LRESULT Window::wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// Equivalent to the this pointer of the Window class.
	static Window* window;

	switch (message) {
		[[unlikely]] case WM_NCCREATE:
			window = reinterpret_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);
			window->hwnd = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
			break;

		// It shouldnt recive WM_HOTKEY on non registered hot keys.
		case WM_HOTKEY: {
			Screenshot screenshot;
			switch (wparam) {
				case SMSS_HOTKEY_PRINTSCREEN:
					screenshot.fullscreen();
					break;
				case SMSS_HOTKEY_CTRL_PRINTSCREEN:
					screenshot.window();
					break;
				case SMSS_HOTKEY_ALT_PRINTSCREEN:
					screenshot.client();
					break;
			}
			return 0;
		}

		case SMSS_WM_APP_NOTIFYCALLBACK:
			if (LOWORD(lparam) == WM_CONTEXTMENU) {
				const POINT point{ LOWORD(wparam), HIWORD(wparam) };
				window->show_menu(point);
			}
			return 0;
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case ID_AUTOSTART:
					g_config.set_autostart();
					break;
				case ID_FOLDER:
					g_config.set_directory();
					break;
				case ID_FORMAT_PNG:
					g_config.set_format(SMSS_FORMAT_PNG);
					break;
				case ID_FORMAT_BMP:
					g_config.set_format(SMSS_FORMAT_BMP);
					break;
				case ID_EXIT:
					DestroyWindow(hwnd);
			}
			return 0;
		case WM_DESTROY: {

			// Destroy tray icon.
			NOTIFYICONDATAW notifyicondata{
				.hWnd{ hwnd },
				.uID{ IDI_ICON }
			};
			smss_assert(Shell_NotifyIconW(NIM_DELETE, &notifyicondata), != 0);
			
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProcW(hwnd, message, wparam, lparam);
}

void Window::create_tray_icon()
{
	NOTIFYICONDATAW notifyicondata{
		.cbSize{ sizeof(NOTIFYICONDATAW) },
		.hWnd{ hwnd },
		.uID{ IDI_ICON },
		.uFlags{ NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP },
		.uCallbackMessage{ SMSS_WM_APP_NOTIFYCALLBACK },
		.hIcon{ LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_ICON)) },
		.uVersion{ NOTIFYICON_VERSION_4 },
	};

	//initialize .szTip
	std::wstring(&w)(int) { std::to_wstring };
	const std::wstring tip{ L"Single Menu Screenshot " + w(SMSS_VERSION_NUMBER_MAJOR) + L"." + w(SMSS_VERSION_NUMBER_MINOR) + L"." + w(SMSS_VERSION_NUMBER_PATCH) };
	std::wcscpy(notifyicondata.szTip, tip.c_str());

	smss_assert(Shell_NotifyIconW(NIM_ADD, &notifyicondata), != 0);
	smss_assert(Shell_NotifyIconW(NIM_SETVERSION, &notifyicondata), != 0);
}

//hotkeys have to be set with RegisterHotKey()
void Window::register_hotkeys() const noexcept
{
	smss_assert(RegisterHotKey(hwnd, SMSS_HOTKEY_PRINTSCREEN, MOD_NOREPEAT, VK_SNAPSHOT), != 0);
	smss_assert(RegisterHotKey(hwnd, SMSS_HOTKEY_ALT_PRINTSCREEN, MOD_ALT | MOD_NOREPEAT, VK_SNAPSHOT), != 0);
	smss_assert(RegisterHotKey(hwnd, SMSS_HOTKEY_CTRL_PRINTSCREEN, MOD_CONTROL | MOD_NOREPEAT, VK_SNAPSHOT), != 0);
}

void Window::show_menu(const POINT& point) const noexcept
{
	const auto hmenu{ LoadMenuW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDR_MENU)) };
	const auto sub_menu{ GetSubMenu(hmenu, 0) };
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
	smss_assert(DestroyMenu(hmenu), != 0);
}

//sets check marks on menu items
void Window::set_check_menu_items(HMENU hmenu) const noexcept
{ 
	if (g_config.format == SMSS_FORMAT_PNG)
		CheckMenuItem(hmenu, ID_FORMAT_PNG, MF_CHECKED);
	if (g_config.format == SMSS_FORMAT_BMP)
		CheckMenuItem(hmenu, ID_FORMAT_BMP, MF_CHECKED);
	if (g_config.autostart)
		CheckMenuItem(hmenu, ID_AUTOSTART, MF_CHECKED);
}
