#include "pch.h"
#include "config.h"
#include "helpers.h"

/* Config example

top_level_key=value
top_level_key=value
top_level_key=value0,value1,value2
#section
##subsection
subsection_key=value
subsection_key=value
subsection_key=value
##end
##subsection
subsection_key=value0,value1
subsection_key=value
subsection_key=value
##end
#end
#section
section_key=value
section_key=value
section_key=value
#end

*/

namespace
{
	constexpr auto SMSS_REG_SUBKEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	constexpr auto SMSS_REG_VALUE = L"SingleMenuScreenshot";
}

void Config::read()
{
	std::ifstream file(get_path());
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			auto pos{ line.find('=') };
			if (pos != std::string::npos) {
				read_top_level(line.substr(0, pos), line.substr(pos + 1));
			}
		}
	}
	else {
		set_defaults();
		write();
	}
}

void Config::set_autostart()
{
	if (get_autostart()) {
		smss_assert(RegDeleteKeyValueW(HKEY_CURRENT_USER, SMSS_REG_SUBKEY, SMSS_REG_VALUE), == ERROR_SUCCESS);
	}
	else {
		HKEY hkey;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, SMSS_REG_SUBKEY, 0, KEY_ALL_ACCESS, &hkey) == ERROR_SUCCESS) {
			wchar_t path[MAX_PATH];
			GetModuleFileNameW(nullptr, path, MAX_PATH);
			if (RegSetValueExW(hkey, SMSS_REG_VALUE, 0, REG_SZ, reinterpret_cast<BYTE*>(path), sizeof(path)) == ERROR_SUCCESS) {
				smss_assert(RegCloseKey(hkey), == ERROR_SUCCESS);
			}
		}
	}
}

bool Config::get_autostart()
{	
	// First get required buffer size.
	DWORD buffer_size = 0;
	if (RegGetValueW(HKEY_CURRENT_USER, SMSS_REG_SUBKEY, SMSS_REG_VALUE, RRF_RT_REG_SZ, NULL, NULL, &buffer_size) == ERROR_SUCCESS) {
		
		auto data = std::make_unique<wchar_t[]>(buffer_size);
		if (RegGetValueW(HKEY_CURRENT_USER, SMSS_REG_SUBKEY, SMSS_REG_VALUE, RRF_RT_REG_SZ, NULL, data.get(), &buffer_size) == ERROR_SUCCESS) {
			wchar_t path[MAX_PATH];
			GetModuleFileNameW(nullptr, path, MAX_PATH);
			if (!wcscmp(data.get(), path)) {
				return true;
			}
		}
	}
	return false;
}

void Config::set_format(SMSS_FORMAT_ format)
{
	this->format.val = format;
	write();
}

void Config::set_directory()
{
	smss_assert(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE), >= S_OK);
	Microsoft::WRL::ComPtr<IFileDialog> file_dialog;
	if (CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(file_dialog.ReleaseAndGetAddressOf())) == S_OK) {
		DWORD options;
		smss_assert(file_dialog->GetOptions(&options), == S_OK);

		// Set options to allow us to pick a directory.
		// Show the dialog.
		smss_assert(file_dialog->SetOptions(options | FOS_PICKFOLDERS), == S_OK);

		if (file_dialog->Show(nullptr) == S_OK) {

			//after we pick directory get us its path and set the config
			Microsoft::WRL::ComPtr<IShellItem> shell_item;
			if (file_dialog->GetResult(shell_item.ReleaseAndGetAddressOf()) == S_OK) {
				LPWSTR path;
				if (shell_item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path) == S_OK) {
					directory.val = path;
					write();
				}
				CoTaskMemFree(path);
			}
		}
	}
	CoUninitialize();
}

void Config::write()
{
	std::ofstream file(get_path());
	write_top_level(file);
}

void Config::set_defaults() noexcept
{
	format.val = SMSS_FORMAT_PNG;
	directory.val = get_desktop();
}

void Config::read_top_level(const std::string& key, const std::string& val)
{
	if (key == format.key) {
		strtoval(val, format.val);
		return;
	}
	if (key == directory.key) {
		directory.val = val;
		return;
	}
}

void Config::write_top_level(std::ofstream& file) const
{
	// Have to cast to prevent writing as "char" instead of number.
	file << format.key << '=' << static_cast<int>(format.val) << '\n';

	file << directory.key << '=' << directory.val.string() << '\n';
}

std::filesystem::path Config::get_desktop() const noexcept
{
	PWSTR desktop;
	smss_assert(SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &desktop), == S_OK);
	return std::filesystem::path(std::move(desktop));
}

std::filesystem::path Config::get_path() const noexcept
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(nullptr, path, MAX_PATH);
	return std::filesystem::path(path).parent_path() / L"config.txt";
}
