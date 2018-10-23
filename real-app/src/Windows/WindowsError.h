#pragma once

#include "../ExpectedError.h"

namespace miniant::Windows {

class WindowsError : public ExpectedError {
public:
    WindowsError();

    WindowsError(std::string message) noexcept:
        ExpectedError(std::move(message)) {}

    WindowsError(const char* message):
        ExpectedError(message) {}
};

}
