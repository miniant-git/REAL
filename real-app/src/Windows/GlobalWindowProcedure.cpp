#include "GlobalWindowProcedure.h"

#include "Exception.h"

using namespace miniant::Windows;

std::unordered_map<HWND, GlobalWindowProcedure::WindowProcedure> GlobalWindowProcedure::s_windowProcedureMap;

UINT GlobalWindowProcedure::GetFreeEventId() noexcept {
    static UINT nextFreeEventId = WM_USER;
    return nextFreeEventId++;
}

WNDCLASS GlobalWindowProcedure::RegisterWindowClass(LPCTSTR lpszClassName) {
    WNDCLASS wc = {};
    wc.lpszClassName = lpszClassName;
    wc.lpfnWndProc = &WndProc;
    if (!::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(&WndProc), &wc.hInstance))
        throw Exception();

    if (::RegisterClass(&wc) == 0)
        throw Exception();

    return wc;
}

void GlobalWindowProcedure::SetWindowProcedure(HWND hWnd, WindowProcedure procedure) {
    s_windowProcedureMap[hWnd] = procedure;
}

LRESULT CALLBACK GlobalWindowProcedure::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WindowProcedure* windowProcedure;
    try {
        windowProcedure = &s_windowProcedureMap.at(hWnd);
    } catch (std::out_of_range&) {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    if (*windowProcedure) {
        std::optional<LRESULT> lResult = (*windowProcedure)(hWnd, uMsg, wParam, lParam);
        if (lResult)
            return lResult.value();
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
