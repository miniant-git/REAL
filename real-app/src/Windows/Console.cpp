#include "Console.h"

#include <Windows.h>

#include <iostream>

using namespace miniant::Windows;

Console::Console(std::function<void()> onShow):
    m_onShow(std::move(onShow)) {}

void Console::Open() {
    if (m_opened) {
        return;
    }

    ::AllocConsole();

    FILE* dummy;
    freopen_s(&dummy, "conout$", "w", stdout);
    freopen_s(&dummy, "conin$", "r", stdin);

    std::cout.clear();
    std::cin.clear();

    if (m_onShow) {
        m_onShow();
    }

    m_opened = true;
}

void Console::Close() {
    if (!m_opened) {
        return;
    }

    ::SendMessage(::GetConsoleWindow(), WM_CLOSE, 0, 0);
    ::FreeConsole();

    m_opened = false;
}
