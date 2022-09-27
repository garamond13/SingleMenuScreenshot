module;
#include "framework.h"

export module screenshot;
import config;

export class Screenshot {
public:
	Screenshot(const Config* config) : compatible_hdc(), compatible_bitmap()
	{
		this->config = config;
		set_format_and_path();
	}

	void fullscreen()
	{
		//get device context (dc) of the entire screen
		auto hdc{ GetDC(nullptr) };

		//get all necessary screen dimensions
		//the minimum supported windows version for below functions is win 10 1607
		auto dpi{ GetDpiForSystem() };
		auto width{ GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, dpi) };
		auto height{ GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, dpi) };
		auto left{ GetSystemMetricsForDpi(SM_XVIRTUALSCREEN, dpi) };
		auto top{ GetSystemMetricsForDpi(SM_YVIRTUALSCREEN, dpi) };

		//create dc that we can draw to
		compatible_hdc = CreateCompatibleDC(hdc);
		compatible_bitmap = CreateCompatibleBitmap(hdc, width, height);

		//capture the image
		SelectObject(compatible_hdc, compatible_bitmap);
		BitBlt(compatible_hdc, 0, 0, width, height, hdc, left, top, SRCCOPY | CAPTUREBLT);

		ReleaseDC(nullptr, hdc);
		save();
	}

	//captures fullscreen than crops bitmap to dimensions and position of the active window
	void window()
	{
		//get device context (dc) of the entire screen
		auto hdc{ GetDC(nullptr) };

		//get active window dimensions
		auto hwnd{ GetForegroundWindow() };
		RECT rect;
		DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));

		//create dc that we can draw to
		compatible_hdc = CreateCompatibleDC(hdc);
		compatible_bitmap = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);

		//capture the image
		SelectObject(compatible_hdc, compatible_bitmap);
		BitBlt(compatible_hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdc, rect.left, rect.top, SRCCOPY | CAPTUREBLT);

		ReleaseDC(nullptr, hdc);
		save();
	}

	//captures only the client area of the currently active window
	//sometimes it can give black image with correct dimensions, at the same time window function gives same dimesions and normal image
	void client()
	{
		auto hwnd{ GetForegroundWindow() };
		auto hdc{ GetDC(hwnd) };
		RECT rect;
		GetClientRect(hwnd, &rect);

		//create dc that we can draw to
		compatible_hdc = CreateCompatibleDC(hdc);
		compatible_bitmap = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);

		//capture the image
		SelectObject(compatible_hdc, compatible_bitmap);
		BitBlt(compatible_hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdc, 0, 0, SRCCOPY);

		ReleaseDC(hwnd, hdc);
		save();
	}

	~Screenshot() noexcept
	{
		DeleteDC(compatible_hdc);
		DeleteObject(compatible_bitmap);
	}

private:
	void save()
	{
		ULONG_PTR gdiplus_token;
		Gdiplus::GdiplusStartupInput gdiplus_startup_input;
		if (Gdiplus::GdiplusStartup(&gdiplus_token, &gdiplus_startup_input, nullptr) == Gdiplus::Status::Ok) {
			Gdiplus::Bitmap bitmap(compatible_bitmap, nullptr);
			CLSID clsid; //an image encoder thats gonna be used to save the image (png encoder, jpg encoder, etc.)
			get_encoder_clsid(&clsid);
			bitmap.Save(path.c_str(), &clsid, nullptr);
		}
		//before calling this all gdi+ objects must be destroyed or out of scope
		Gdiplus::GdiplusShutdown(gdiplus_token);
	}

	void get_encoder_clsid(CLSID* clsid)
	{
		UINT count; //number of image encoders
		UINT size; //size of the image encoder array in bytes
		Gdiplus::GetImageEncodersSize(&count, &size);
		auto image_codec_info{ std::make_unique<Gdiplus::ImageCodecInfo[]>(size) };
		GetImageEncoders(count, size, image_codec_info.get());
		for (int i{}; i < count; ++i)
			if (wcscmp(image_codec_info[i].MimeType, format) == 0)
				*clsid = image_codec_info[i].Clsid;
	}

	void set_format_and_path()
	{
		path = config->directory;
		std::chrono::zoned_time time(std::chrono::current_zone(), std::chrono::system_clock::now()); //get the current local time
		path += L"\\" + std::format(L"{:%Y-%m-%d %H.%M }", time) + random_wstring(5);
		if (config->flags & Config::FORMAT_PNG) {
			path += L".png";
			format = L"image/png";
		}
		else if (config->flags & Config::FORMAT_BMP) {
			path += L".bmp";
			format = L"image/bmp";
		}
	}

	const std::wstring random_wstring(size_t lenght)
	{
		std::wstring wstring(lenght, 0);
		std::random_device seed;
		std::mt19937 generator(seed());
		constexpr wchar_t set[]{ L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" };
		std::uniform_int_distribution<std::mt19937::result_type> distribution(0, std::char_traits<wchar_t>::length(set) - 1); //distribution in range from set[first] to set[last], exclude '\0'
		std::generate_n(wstring.begin(), lenght, [&]() { return set[distribution(generator)]; });
		return wstring;
	}

	std::filesystem::path path;
	const wchar_t* format;
	const Config* config;
	HDC compatible_hdc;
	HBITMAP compatible_bitmap;
};
