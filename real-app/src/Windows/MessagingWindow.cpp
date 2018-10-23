#include "MessagingWindow.h"

#include "GlobalWindowProcedure.h"

#include <cassert>

using namespace miniant::Windows;

constexpr TCHAR CLASS_NAME[] = TEXT("MessagingWindow");

tl::expected<HWND, WindowsError> CreateNewWindow() {
    tl::expected<WNDCLASS, WindowsError> wc = GlobalWindowProcedure::RegisterWindowClass(CLASS_NAME);
    if (!wc) {
        return tl::make_unexpected(std::move(wc.error()));
    }

    HWND hWnd = ::CreateWindowEx(
        0,
        CLASS_NAME,
        NULL,
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        NULL,
        wc->hInstance,
        NULL);
    if (hWnd == NULL) {
        return tl::make_unexpected(WindowsError());
    }

    return hWnd;
}

tl::expected<MessagingWindow, WindowsError> MessagingWindow::Create() {
    tl::expected hWnd = CreateNewWindow();
    if (!hWnd) {
        return tl::make_unexpected(hWnd.error());
    }

    return MessagingWindow(*hWnd);
}

tl::expected<std::unique_ptr<MessagingWindow>, WindowsError> MessagingWindow::CreatePtr() {
    tl::expected hWnd = CreateNewWindow();
    if (!hWnd) {
        return tl::make_unexpected(hWnd.error());
    }

    return std::unique_ptr<MessagingWindow>(new MessagingWindow(*hWnd));
}

MessagingWindow::MessagingWindow(HWND hWnd):
    m_hWnd(hWnd) {
    assert(m_hWnd != NULL);

    GlobalWindowProcedure::SetWindowProcedure(m_hWnd, [this](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        auto eventHandler = m_eventHandlerMap.find(uMsg);
        if (eventHandler != m_eventHandlerMap.end()) {
            if (eventHandler->second) {
                return eventHandler->second(*this, wParam, lParam);
            }
        }

        return std::optional<LRESULT>();
        });
}

MessagingWindow::MessagingWindow(MessagingWindow&& other):
    m_hWnd(other.m_hWnd),
    m_eventHandlerMap(std::move(other.m_eventHandlerMap)) {
    assert(other.m_hWnd != NULL);

    other.m_hWnd = NULL;

    GlobalWindowProcedure::SetWindowProcedure(m_hWnd, [this](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        auto eventHandler = m_eventHandlerMap.find(uMsg);
        if (eventHandler != m_eventHandlerMap.end()) {
            if (eventHandler->second) {
                return eventHandler->second(*this, wParam, lParam);
            }
        }

        return std::optional<LRESULT>();
        });
}

MessagingWindow::~MessagingWindow() {
    if (m_hWnd != NULL) {
        GlobalWindowProcedure::SetWindowProcedure(m_hWnd, nullptr);
    }
}

MessagingWindow& MessagingWindow::operator= (MessagingWindow&& rhs) {
    assert(rhs.m_hWnd != NULL);

    GlobalWindowProcedure::SetWindowProcedure(m_hWnd, nullptr);

    m_hWnd = rhs.m_hWnd;
    m_eventHandlerMap = std::move(rhs.m_eventHandlerMap);
    rhs.m_hWnd = NULL;

    GlobalWindowProcedure::SetWindowProcedure(m_hWnd, [this](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        auto eventHandler = m_eventHandlerMap.find(uMsg);
        if (eventHandler != m_eventHandlerMap.end()) {
            if (eventHandler->second) {
                return eventHandler->second(*this, wParam, lParam);
            }
        }

        return std::optional<LRESULT>();
        });

    return *this;
}

HWND MessagingWindow::GetHWindow() noexcept {
    assert(m_hWnd != NULL);
    return m_hWnd;
}

void MessagingWindow::SetEventHandler(UINT event, EventHandler handler) {
    assert(m_hWnd != NULL);
    assert(handler);
    m_eventHandlerMap[event] = std::move(handler);
}
