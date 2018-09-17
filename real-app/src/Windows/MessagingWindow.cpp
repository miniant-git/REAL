#include "MessagingWindow.h"

#include "Exception.h"
#include "GlobalWindowProcedure.h"

using namespace miniant::Windows;

constexpr TCHAR CLASS_NAME[] = TEXT("MessagingWindow");

MessagingWindow::MessagingWindow() {
    m_hWnd = ::CreateWindowEx(
        0,
        CLASS_NAME,
        NULL,
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        NULL,
        GlobalWindowProcedure::RegisterWindowClass(CLASS_NAME).hInstance,
        NULL);
    if (m_hWnd == NULL)
        throw Exception();

    GlobalWindowProcedure::SetWindowProcedure(m_hWnd, [this](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        auto eventHandler = m_eventHandlerMap.find(uMsg);
        if (eventHandler != m_eventHandlerMap.end()) {
            if (eventHandler->second)
                return eventHandler->second(*this, wParam, lParam);
        }

        return std::optional<LRESULT>();
        });
}

MessagingWindow::~MessagingWindow() noexcept {
    GlobalWindowProcedure::SetWindowProcedure(m_hWnd, nullptr);
}

HWND MessagingWindow::GetHWindow() noexcept {
    return m_hWnd;
}

void MessagingWindow::SetEventHandler(UINT event, EventHandler handler) {
    m_eventHandlerMap[event] = std::move(handler);
}
