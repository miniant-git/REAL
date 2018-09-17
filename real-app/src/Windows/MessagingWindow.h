#pragma once

#include <Windows.h>

#include <functional>
#include <map>
#include <optional>

namespace miniant::Windows {

class MessagingWindow {
public:
    using EventHandler = std::function<std::optional<LRESULT>(MessagingWindow&, WPARAM, LPARAM)>;

    MessagingWindow();
    ~MessagingWindow() noexcept;

    HWND GetHWindow() noexcept;

    void SetEventHandler(UINT event, EventHandler handler);

private:
    HWND m_hWnd;
    std::unordered_map<UINT, EventHandler> m_eventHandlerMap;
};

}
