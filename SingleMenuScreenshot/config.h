#pragma once

#include "pch.h"
#include "config_pairs.h"

enum SMSS_FORMAT_
{
	SMSS_FORMAT_PNG,
	SMSS_FORMAT_BMP
};

class Config
{
	using flag_type = uint8_t;
public:
	void read();
	void set_autostart();
	void set_directory();
	void set_format(SMSS_FORMAT_ format);
	flag_type SMSS_NAME_FORMAT_VAL;
	bool SMSS_NAME_AUTOSTART_VAL;

	//the directory in which screenshots will be saved
	std::filesystem::path directory;

private:
	void write();
	void set_defaults() noexcept;
	void read_top_level(const std::string& key, const std::string& val);
	void write_top_level(std::ofstream& file) const;
	std::filesystem::path get_desktop() const noexcept;
	std::filesystem::path get_path() const noexcept;
};
