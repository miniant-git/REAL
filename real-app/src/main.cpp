#include "AutoUpdater.h"
#include "OStreamSink.h"

#include "Windows/Console.h"
#include "Windows/MinimumLatencyAudioClient.h"
#include "Windows/MessagingWindow.h"
#include "Windows/TrayIcon.h"

#include "../resource.h"

#include <spdlog/spdlog.h>

#include <conio.h>

#include <locale>
#include <iostream>
#include <sstream>

using namespace miniant::AutoUpdater;
using namespace miniant::Spdlog;
using namespace miniant::Windows;
using namespace miniant::Windows::WasapiLatency;

constexpr Version APP_VERSION(0, 1, 3);
constexpr TCHAR COMMAND_LINE_OPTION_TRAY[] = TEXT("--tray");

void WaitForAnyKey(const std::string& message) {
    while (_kbhit()) {
        _getch();
    }

    spdlog::get("app_out")->info(message);
    _getch();
}

void DisplayExitMessage(int errorCode) {
    if (errorCode == 0) {
        WaitForAnyKey("\nPress any key to disable and exit . . .");
    } else {
        WaitForAnyKey("\nPress any key to exit . . .");
    }
}

std::string ToLower(const std::string& string) {
    std::string result;
    for (const auto& c : string) {
        result.append(1, std::tolower(c, std::locale()));
    }

    return result;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    auto oss = std::make_shared<std::ostringstream>();
    auto sink = std::make_shared<OStreamSink>(oss, true);
    auto app_out = std::make_shared<spdlog::logger>("app_out", sink);
    app_out->set_pattern("%v");
    spdlog::register_logger(app_out);
    app_out->info("Minimum audio latency enabled on the DEFAULT playback device!\n");

    auto console = std::make_shared<Console>([=] {
        std::lock_guard<std::mutex> lock(sink->GetMutex());
        std::cout << oss->str();
        std::cout.flush();
        oss->set_rdbuf(std::cout.rdbuf());
        });

    std::unique_ptr<MessagingWindow> window;
    std::unique_ptr<TrayIcon> trayIcon;

    std::wstring commandLine(pCmdLine);
    int errorCode = 0;

    if (commandLine == COMMAND_LINE_OPTION_TRAY) {
        tl::expected windowPtrResult = MessagingWindow::CreatePtr();
        if (!windowPtrResult) {
#pragma push_macro("GetMessage")
#undef GetMessage
            app_out->error("Error: {}", windowPtrResult.error().GetMessage());
            return errorCode;
        }

        window = std::move(*windowPtrResult);
        HICON hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
        trayIcon = std::make_unique<TrayIcon>(*window, hIcon);
        trayIcon->SetLButtonUpHandler([=, &errorCode](TrayIcon& trayIcon) {
            trayIcon.Hide();
            console->Open();

            DisplayExitMessage(errorCode);

            console->Close();
            return std::optional<LRESULT>();
            });
        trayIcon->Show();
    } else {
        console->Open();
    }

    app_out->info("REAL - REduce Audio Latency {}, mini)(ant, 2018", APP_VERSION.ToString());
    app_out->info("Project: https://github.com/miniant-git/REAL\n");

    errorCode = MinimumLatencyAudioClient().Start();
    if (errorCode == 0) {
        app_out->info("Minimum audio latency enabled on the DEFAULT playback device!\n");
    } else {
        app_out->info("ERROR: Could not enable low-latency mode. Error code {}.\n", errorCode);
    }

    app_out->info("Checking for updates...");
    AutoUpdater updater;
    tl::expected cleanupResult = updater.CleanupPreviousSetup();
    if (!cleanupResult) {
        app_out->info("Error: {}", cleanupResult.error().GetMessage());
    }

    tl::expected info = updater.GetUpdateInfo();
    if (!info) {
        app_out->info("Error: {}", info.error().GetMessage());
        app_out->info("Update failed!");
    } else if (info->version > APP_VERSION) {
        if (trayIcon != nullptr) {
            trayIcon->Hide();
        }

        console->Open();

        app_out->info("A new update is available!");
        if (info->releaseNotes) {
            app_out->info(*info->releaseNotes);
        }

        std::cout << "Do you want to update to " << info->version.ToString() << "? [y/N] : ";
        char line[5];
        std::cin.getline(line, 5);
        std::string prompt(ToLower(line));
        if (prompt == "y" || prompt == "yes") {
            updater.ApplyUpdate(*info);
        } else {
            app_out->info("No: Keeping the current version.");
        }

        DisplayExitMessage(errorCode);
        return errorCode;
    } else {
        app_out->info("The application is up-to-date.");
    }

#pragma pop_macro("GetMessage")
    if (commandLine == COMMAND_LINE_OPTION_TRAY) {
        MSG msg;
        while (::GetMessage(&msg, NULL, 0, 0) > 0) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    } else {
        DisplayExitMessage(errorCode);
    }

    return errorCode;
}
