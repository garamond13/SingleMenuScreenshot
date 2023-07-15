#include "pch.h"
#include "config.h"
#include "config_pairs.h"
#include "helpers.h"

/* config example

top_level_key=top_level_value
top_level_key=top_level_value
top_level_key=top_level_value
#section
##subsection
subsection_key=subsection_value
subsection_key=subsection_value
subsection_key=subsection_value
##end
##subsection
subsection_key=subsection_value
subsection_key=subsection_value
subsection_key=subsection_value
##end
#end
#section
section_key=section_value
section_key=section_value
section_key=section_value
#end

*/

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
	const auto subkey{ L"Software\\Microsoft\\Windows\\CurrentVersion\\Run" };
	const auto value{ L"SingleMenuScreenshot" };
	if (SMSS_NAME_AUTOSTART_VAL) {
		smss_assert(RegDeleteKeyValueW(HKEY_CURRENT_USER, subkey, value), == ERROR_SUCCESS);
		autostart = false;
	}
	else {

		//note that it takes the current path
		//if the program changes the path it wont autostart from the new path, but the config will be still set to autostart
		wchar_t path[MAX_PATH];
		GetModuleFileNameW(nullptr, path, MAX_PATH);

		HKEY hkey;
		smss_assert(RegOpenKeyExW(HKEY_CURRENT_USER, subkey, 0, KEY_ALL_ACCESS, &hkey), == ERROR_SUCCESS);
		smss_assert(RegSetValueExW(hkey, value, 0, REG_SZ, reinterpret_cast<BYTE*>(path), sizeof(path)), == ERROR_SUCCESS);
		smss_assert(RegCloseKey(hkey), == ERROR_SUCCESS);
		autostart = true;
	}
	write();
}

void Config::set_format(SMSS_FORMAT_ format)
{
	this->format = format;
	write();
}

void Config::set_directory()
{
	smss_assert(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE), >= S_OK);
	Microsoft::WRL::ComPtr<IFileDialog> file_dialog;
	if (CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(file_dialog.ReleaseAndGetAddressOf())) == S_OK) {
		DWORD options;
		smss_assert(file_dialog->GetOptions(&options), == S_OK);

		//set options to allow us to pick a directory
		//show the dialog
		smss_assert(file_dialog->SetOptions(options | FOS_PICKFOLDERS), == S_OK);

		if (file_dialog->Show(nullptr) == S_OK) {

			//after we pick directory get us its path and set the config
			Microsoft::WRL::ComPtr<IShellItem> shell_item;
			if (file_dialog->GetResult(shell_item.ReleaseAndGetAddressOf()) == S_OK) {
				LPWSTR path;
				if (shell_item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path) == S_OK) {
					directory = path;
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
	SMSS_NAME_FORMAT_VAL = SMSS_FORMAT_PNG;
	SMSS_NAME_AUTOSTART_VAL = false;
	directory = get_desktop();
}

void Config::read_top_level(const std::string& key, const std::string& val)
{
	if (key == SMSS_NAME_FORMAT_KEY) {
		strtoval(val, SMSS_NAME_FORMAT_VAL);
		return;
	}
	if (key == SMSS_NAME_AUTOSTART_KEY) {
		strtoval(val, SMSS_NAME_AUTOSTART_VAL);
		return;
	}
	if (key == SMSS_NAME_DIRECTORY_KEY) {
		SMSS_NAME_DIRECTORY_VAL = val;
		return;
	}
}

void Config::write_top_level(std::ofstream& file) const
{
	//have to cast to prevent writing char as "char" instead of number
	file << SMSS_NAME_FORMAT_KEY << '=' << static_cast<uint16_t>(SMSS_NAME_FORMAT_VAL) << '\n';

	file << SMSS_NAME_AUTOSTART_KEY << '=' << SMSS_NAME_AUTOSTART_VAL << '\n';
	file << SMSS_NAME_DIRECTORY_KEY << '=' << SMSS_NAME_DIRECTORY_VAL.string() << '\n';
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
