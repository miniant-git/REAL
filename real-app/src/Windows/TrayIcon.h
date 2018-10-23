#pragma once

#include "MessagingWindow.h"

namespace miniant::Windows {

class TrayIcon {
public:
    using TrayEventHandler = std::function<std::optional<LRESULT>(TrayIcon&)>;

    TrayIcon(MessagingWindow& window, HICON hIcon);
    ~TrayIcon() noexcept;

    void Show();
    void Hide();

    void SetLButtonUpHandler(TrayEventHandler handler) noexcept;

private:
    NOTIFYICONDATA m_data;
    MessagingWindow& m_window;
    TrayEventHandler m_lButtonUpHandler;
    bool m_shown = false;
};

}
