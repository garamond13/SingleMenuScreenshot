#include "pch.h"
#include "global.h"
#include "window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance, [[maybe_unused]] _In_ LPWSTR lpCmdLine, [[maybe_unused]] _In_ int nCmdShow)
{
    // Only allow single app instance.
    if (!CreateMutexW(nullptr, TRUE, L"33A4F83D-364C-4EF4-89EA-B6BD54376253" /* MUTEX_GUID, generated with Microsoft Visual Studio->Tools->Create GUID */) || GetLastError() == ERROR_ALREADY_EXISTS)
        return 1;

    g_config.read();
    auto window{ std::make_unique<Window>(hInstance) };

    return window->message_loop();
}