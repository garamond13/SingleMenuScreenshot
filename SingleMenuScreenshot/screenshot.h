#pragma once

#include "pch.h"

class Screenshot
{
public:
	Screenshot();
	~Screenshot();
	void fullscreen();

	//captures fullscreen than crops bitmap to dimensions and position of the active window
	void window();

	//captures only the client area of the currently active window
	//sometimes it can give black image with correct dimensions, at the same time window function gives same dimesions and normal image
	void client();

private:
	void save();
	CLSID get_encoder();
	std::wstring get_random_wstring(size_t lenght);
	std::filesystem::path path{};
	const wchar_t* format{};
	HDC compatible_hdc{};
	HBITMAP compatible_bitmap{};
};
