#pragma once

#include "pch.h"
#include "helpers.h"

enum SMSS_FORMAT_ : uint8_t
{
	SMSS_FORMAT_PNG,
	SMSS_FORMAT_BMP
};

template<typename T, Char_array str>
struct Config_pair
{
	T val;
	static constexpr const char* key{ str.val.data() };
};

class Config
{
public:
	void read();
	void set_autostart();
	bool get_autostart();
	void set_directory();
	void set_format(SMSS_FORMAT_ format);
	Config_pair<uint8_t, "fmt"> format;
	Config_pair< std::filesystem::path, "dir"> directory; // The directory in which screenshots will be saved.
private:
	void write();
	void set_defaults() noexcept;
	void read_top_level(const std::string& key, const std::string& val);
	void write_top_level(std::ofstream& file) const;
	std::filesystem::path get_desktop() const noexcept;
	std::filesystem::path get_path() const noexcept;
};
