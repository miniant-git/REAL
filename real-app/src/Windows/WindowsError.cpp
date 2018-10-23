#include "WindowsError.h"

#include <Windows.h>

using namespace miniant::Windows;

std::string GetErrorMessage() {
    DWORD lastError = ::GetLastError();
    return "Last error: " + std::to_string(lastError);
}

WindowsError::WindowsError():
    ExpectedError(GetErrorMessage()) {}
