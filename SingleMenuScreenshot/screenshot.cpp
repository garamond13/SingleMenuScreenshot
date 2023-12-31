#include "pch.h"
#include "screenshot.h"
#include "global.h"
#include "helpers.h"

Screenshot::Screenshot()
{
	path = g_config.directory.val;
	const std::chrono::zoned_time time(std::chrono::current_zone(), std::chrono::system_clock::now());
	path /= std::format(L"{:%Y-%m-%d %H_%M }", time) + get_random_wstring(5);
	if (g_config.format.val == SMSS_FORMAT_PNG) {
		path.replace_extension(L".png");
		format = L"image/png";
	}
	else if (g_config.format.val == SMSS_FORMAT_BMP) {
		path.replace_extension(L".bmp");
		format = L"image/bmp";
	}
}

// Dpi functions, the minimum supported windows version is win 10 1607.
void Screenshot::fullscreen()
{
	// Get device context (dc) of the entire screen.
	auto hdc{ GetDC(nullptr) };

	// Get screen dimensions.
	const auto dpi{ GetDpiForSystem() };
	const auto width{ GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, dpi) };
	const auto height{ GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, dpi) };

	// Create dc that we can draw to.
	auto compatible_hdc{ CreateCompatibleDC(hdc) };
	auto compatible_bitmap{ CreateCompatibleBitmap(hdc, width, height) };

	// Capture the image.
	SelectObject(compatible_hdc, compatible_bitmap);
	smss_assert(BitBlt(compatible_hdc, 0, 0, width, height, hdc, GetSystemMetricsForDpi(SM_XVIRTUALSCREEN, dpi), GetSystemMetricsForDpi(SM_YVIRTUALSCREEN, dpi), SRCCOPY | CAPTUREBLT), != 0);

	smss_assert(ReleaseDC(nullptr, hdc), == 1);
	save(compatible_bitmap);

	// Free resources.
	DeleteDC(compatible_hdc);
	DeleteObject(compatible_bitmap);
}

void Screenshot::window()
{
	// Get device context (dc) of the entire screen.
	auto hdc{ GetDC(nullptr) };

	// Get active window dimensions.
	auto hwnd{ GetForegroundWindow() };
	RECT rect;
	smss_assert(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT)), == S_OK);

	// Create dc that we can draw to.
	auto compatible_hdc{ CreateCompatibleDC(hdc) };
	auto compatible_bitmap{ CreateCompatibleBitmap(hdc, rc_w<int>(rect), rc_h<int>(rect))};

	// Capture the image.
	SelectObject(compatible_hdc, compatible_bitmap);
	smss_assert(BitBlt(compatible_hdc, 0, 0, rc_w<int>(rect), rc_h<int>(rect), hdc, rect.left, rect.top, SRCCOPY | CAPTUREBLT), != 0);

	smss_assert(ReleaseDC(nullptr, hdc), == 1);
	save(compatible_bitmap);

	// Free resources.
	DeleteDC(compatible_hdc);
	DeleteObject(compatible_bitmap);
}

void Screenshot::client()
{
	auto hwnd{ GetForegroundWindow() };
	auto hdc{ GetDC(hwnd) };
	RECT rect;
	smss_assert(GetClientRect(hwnd, &rect), != 0);

	// Create dc that we can draw to.
	auto compatible_hdc{ CreateCompatibleDC(hdc) };
	auto compatible_bitmap{ CreateCompatibleBitmap(hdc, rc_w<int>(rect), rc_h<int>(rect)) };

	// Capture the image.
	SelectObject(compatible_hdc, compatible_bitmap);
	smss_assert(BitBlt(compatible_hdc, 0, 0, rc_w<int>(rect), rc_h<int>(rect), hdc, 0, 0, SRCCOPY), != 0);

	smss_assert(ReleaseDC(hwnd, hdc), == 1);
	save(compatible_bitmap);

	// Free resources.
	DeleteDC(compatible_hdc);
	DeleteObject(compatible_bitmap);
}

void Screenshot::save(HBITMAP compatible_bitmap)
{
	ULONG_PTR gdiplus_token;
	Gdiplus::GdiplusStartupInput gdiplus_startup_input;
	if (Gdiplus::GdiplusStartup(&gdiplus_token, &gdiplus_startup_input, nullptr) == Gdiplus::Status::Ok) {
		Gdiplus::Bitmap bitmap(compatible_bitmap, nullptr);
		
		// Get image encoder thats gonna be used to save the image (png encoder, jpg encoder, etc.).
		UINT count;
		UINT size;
		smss_assert(Gdiplus::GetImageEncodersSize(&count, &size), == Gdiplus::Status::Ok);
		auto image_codec_info{ std::make_unique_for_overwrite<Gdiplus::ImageCodecInfo[]>(size) };
		GetImageEncoders(count, size, image_codec_info.get());
		for (UINT i{}; i < count; ++i)
			if (std::wcscmp(image_codec_info[i].MimeType, format) == 0) {
				smss_assert(bitmap.Save(path.c_str(), &image_codec_info[i].Clsid, nullptr), == Gdiplus::Status::Ok);
				break;
			}
	}

	// Before calling this all gdi+ objects must be destroyed or out of scope.
	Gdiplus::GdiplusShutdown(gdiplus_token);
}

std::wstring Screenshot::get_random_wstring(size_t lenght)
{
	std::wstring wstring(lenght, 0);
	std::random_device seed;
	std::mt19937 generator(seed());
	static constinit auto set{ L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" };

	// Distribution in range from set[first] to set[last], exclude '\0'.
	std::uniform_int_distribution<std::mt19937::result_type> distribution(0, std::char_traits<wchar_t>::length(set) - 1);
	
	std::generate_n(wstring.begin(), lenght, [&] { return set[distribution(generator)]; });
	return wstring;
}
