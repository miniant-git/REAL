#include "TrayIcon.h"

#include "Exception.h"
#include "GlobalWindowProcedure.h"

using namespace miniant::Windows;

TrayIcon::TrayIcon(MessagingWindow& window, HICON hIcon):
    m_window(window) {
    m_data = {};
    m_data.cbSize = sizeof(m_data);
    m_data.uVersion = NOTIFYICON_VERSION_4;
    m_data.uFlags = NIF_ICON | NIF_MESSAGE;
    m_data.hWnd = window.GetHWindow();
    m_data.hIcon = hIcon;
    m_data.uID = 1;
    m_data.uCallbackMessage = GlobalWindowProcedure::GetFreeEventId();

    window.SetEventHandler(m_data.uCallbackMessage, [this](const MessagingWindow& window, WPARAM wParam, LPARAM lParam) {
        switch (LOWORD(lParam)) {
            case WM_LBUTTONUP:
                if (m_lButtonUpHandler)
                    m_lButtonUpHandler(*this);
                break;

            default:
                break;
        }

        return std::optional<LRESULT>();
        });
}

TrayIcon::~TrayIcon() noexcept {
    m_window.SetEventHandler(m_data.uCallbackMessage, nullptr);
    ::Shell_NotifyIcon(NIM_DELETE, &m_data);
}

void TrayIcon::Show() {
    if (!::Shell_NotifyIcon(NIM_ADD, &m_data))
        throw Exception();

    if (!::Shell_NotifyIcon(NIM_SETVERSION, &m_data)) {
        ::Shell_NotifyIcon(NIM_DELETE, &m_data);
        throw Exception();
    }
}

void TrayIcon::Hide() {
    if (!::Shell_NotifyIcon(NIM_DELETE, &m_data))
        throw Exception();
}

void TrayIcon::SetLButtonUpHandler(TrayEventHandler handler) noexcept {
    m_lButtonUpHandler = std::move(handler);
}
