#include "Exception.h"

#include <Windows.h>

#include <string>

using namespace miniant::Windows;

std::string GetErrorMessage() noexcept {
    DWORD lastError = ::GetLastError();
    return "Last error: " + std::to_string(lastError);
}

Exception::Exception() noexcept: std::runtime_error(GetErrorMessage()) {}
