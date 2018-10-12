#pragma once

#include <functional>

namespace miniant::Windows {

class Console {
public:
    Console(std::function<void()> onShow);

    void Open();
    void Close();

private:
    std::function<void()> m_onShow;
    bool m_opened = false;
};

}
