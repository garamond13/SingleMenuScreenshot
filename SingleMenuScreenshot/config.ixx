module;
#include "framework.h"

export module config;

export class Config {
public:
	//flags
	//bits dont have to be in any order
	//minimum 16 bits, because if less we cant read the config file with current setup
	using flag_type = uint16_t;
	static constexpr flag_type FORMAT_PNG{ 0x1 };
	static constexpr flag_type FORMAT_BMP{ 0x2 };
	static constexpr flag_type AUTOSTART{ 0x4 };

	void write()
	{
		std::filesystem::path path;
		get_path(path);

		//open file for writeing, than write 2 lines
		//write as .c_str because if not it will put the path in quoute marks, something we dont want
		std::wofstream file(path);
		file << directory.c_str() << std::endl << flags;
	}

	void read()
	{
		std::filesystem::path path;
		get_path(path);

		//chechs if the config.dat exists and if not creates it with default settings
		if (!std::filesystem::exists(path))
			write_defaults(path);

		//open file for reading, than read 2 lines
		std::wifstream file(path);
		file >> directory >> flags;
	}

	void write_defaults(const std::filesystem::path& path)
	{
		PWSTR desktop;
		SHGetKnownFolderPath(FOLDERID_Desktop, 0ul, nullptr, &desktop);

		//set defualt settings
		directory = desktop;
		flags |= FORMAT_PNG;

		//open file for writeing, than write 2 lines
		//for .c_str see above write()
		std::wofstream file(path);
		file << directory.c_str() << std::endl << flags;

		CoTaskMemFree(desktop);
	}

	void set_autostart()
	{
		if (flags & AUTOSTART) {
			RegDeleteKeyValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"SimpleScreenshot");
			flags ^= AUTOSTART;
			write();
		}
		else {
			//note that it takes the current path
			//if the program changes the path it wont autostart from the new path, but the config will be still set to autostart
			wchar_t path[MAX_PATH];
			GetModuleFileNameW(nullptr, path, MAX_PATH);

			HKEY hkey;
			RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0ul, KEY_ALL_ACCESS, &hkey);
			RegSetValueExW(hkey, L"SimpleScreenshot", 0ul, REG_SZ, reinterpret_cast<BYTE*>(path), sizeof(path));
			RegCloseKey(hkey);
			flags |= AUTOSTART;
			write();
		}
	}

	void get_folder()
	{
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		winrt::com_ptr<IFileDialog> file_dialog;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(file_dialog.put())))) {
			DWORD options;
			file_dialog->GetOptions(&options);

			//set options to allow us to pick a directory
			//show the dialog
			if (SUCCEEDED(file_dialog->SetOptions(options | FOS_PICKFOLDERS))) {
				file_dialog->Show(nullptr);

				//after we pick directory get us its path and set the config
				winrt::com_ptr<IShellItem> shell_item;
				if (SUCCEEDED(file_dialog->GetResult(shell_item.put()))) {
					LPWSTR path;
					if (SUCCEEDED(shell_item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path))) {
						directory = path;
						write();
					}
					CoTaskMemFree(path);
				}
			}
		}
		CoUninitialize();
	}

	flag_type flags;
	std::filesystem::path directory; //the directory in wich screenshots will be saved in
private:
	//config path: %USERPROFILE%\AppData\Local\SingleMenuScreenshot\config.dat
	void get_path(std::filesystem::path& path)
	{
		PWSTR local_app_data;
		SHGetKnownFolderPath(FOLDERID_LocalAppData, 0ul, nullptr, &local_app_data);
		path = local_app_data;

		//append %USERPROFILE%\AppData\Local with our directory
		//check first if it exists and if not, create it
		path += L"\\SingleMenuScreenshot";
		if (!std::filesystem::exists(path))
			std::filesystem::create_directory(path);

		//finaly make the full path of the config file
		path += L"\\config.dat";

		CoTaskMemFree(local_app_data);
	}
};
