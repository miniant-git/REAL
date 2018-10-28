#pragma once

#include "MessagingWindow.h"

#include <shellapi.h>

namespace miniant::Windows {

class TrayIcon {
public:
    using TrayEventHandler = std::function<std::optional<LRESULT>(TrayIcon&)>;

    TrayIcon(MessagingWindow& window, HICON hIcon);
    ~TrayIcon() noexcept;

    tl::expected<void, WindowsError> Show();
    tl::expected<void, WindowsError> Hide();

    void SetLButtonUpHandler(TrayEventHandler handler) noexcept;

private:
    NOTIFYICONDATA m_data;
    MessagingWindow& m_window;
    TrayEventHandler m_lButtonUpHandler;
    bool m_shown = false;
};

}
