module;
#include "framework.h"

export module main;
import window;

namespace {
    //generated with Microsoft Visual Studio->Tools->Create GUID
    constexpr wchar_t MUTEX_GUID[]{ L"33A4F83D-364C-4EF4-89EA-B6BD54376253" };
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance, [[maybe_unused]] _In_ LPWSTR lpCmdLine, [[maybe_unused]] _In_ int nCmdShow)
{
    //only allow single app instance
    if (!CreateMutexW(nullptr, TRUE, MUTEX_GUID) || GetLastError() == ERROR_ALREADY_EXISTS)
        return 1;

    static Window window;
    window.initialize(hInstance);

    //message loop
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0u, 0u))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
