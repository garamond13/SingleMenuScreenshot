#pragma once

#define _CRT_SECURE_NO_WARNINGS

//system
//

#include "targetver.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <shellapi.h>
#include <Shlobj.h>
#include <gdiplus.h>
#include <dwmapi.h>
#include <wrl/client.h>

//

//std
#include <fstream>
#include <filesystem>
#include <random>
#include <cassert>
#include <memory>
#include <array>
