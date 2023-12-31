#pragma once

#include "pch.h"

class Screenshot
{
public:
	Screenshot();
	void fullscreen();

	// Captures fullscreen than crops bitmap to dimensions and position of the active window.
	void window();

	// Captures only the client area of the currently active window.
	// Sometimes it can give black image with correct dimensions, at the same time window function gives same dimesions and normal image.
	void client();

private:
	void save(HBITMAP compatible_bitmap);
	std::wstring get_random_wstring(size_t lenght);
	std::filesystem::path path;
	std::wstring format;
};
