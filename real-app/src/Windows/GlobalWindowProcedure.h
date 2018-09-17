#pragma once

#include <Windows.h>

#include <functional>
#include <map>
#include <optional>

namespace miniant::Windows {

class GlobalWindowProcedure {
public:
    using WindowProcedure = std::function<std::optional<LRESULT>(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lparam)>;

    static UINT GetFreeEventId() noexcept;

    static WNDCLASS RegisterWindowClass(LPCTSTR lpszClassName);
    static void SetWindowProcedure(HWND hWnd, WindowProcedure procedure);

private:
    static std::unordered_map<HWND, WindowProcedure> s_windowProcedureMap;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

}
