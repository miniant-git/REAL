#pragma once

#include "WindowsError.h"

#include <tl/expected.hpp>

#include <Windows.h>

#include <functional>
#include <map>
#include <optional>

namespace miniant::Windows {

class MessagingWindow {
public:
    using EventHandler = std::function<std::optional<LRESULT>(MessagingWindow&, WPARAM, LPARAM)>;

    MessagingWindow(MessagingWindow&& other);
    ~MessagingWindow();

    MessagingWindow& operator= (MessagingWindow&& rhs);

    HWND GetHWindow() noexcept;

    void SetEventHandler(UINT event, EventHandler handler);
    void RemoveEventHandler(UINT event);

    static tl::expected<MessagingWindow, WindowsError> Create();
    static tl::expected<std::unique_ptr<MessagingWindow>, WindowsError> CreatePtr();

private:
    MessagingWindow(HWND hWnd);

    HWND m_hWnd;
    std::unordered_map<UINT, EventHandler> m_eventHandlerMap;
};

}
