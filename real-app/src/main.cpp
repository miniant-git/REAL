#include "AutoUpdater.h"
#include "Windows/MinimumLatencyAudioClient.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/ostream_sink.h>

#include <conio.h>
#include <iostream>

using namespace miniant::AutoUpdater;
using namespace miniant::Windows::WasapiLatency;

constexpr Version APP_VERSION(0, 1, 3);

void WaitForAnyKey(const std::string& message) {
    while (_kbhit())
        _getch();
    
    spdlog::get("app_out")->info(message);
    _getch();
}

int main(int argc, char** argv) {
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout);
    auto app_out = std::make_shared<spdlog::logger>("app_out", sink);
    app_out->set_pattern("%v");
    spdlog::register_logger(app_out);

    app_out->info("REAL - REduce Audio Latency {}, mini)(ant, 2018", APP_VERSION.ToString());
    app_out->info("Project: https://github.com/miniant-git/REAL\n");

    int errorCode = MinimumLatencyAudioClient().Start();
    if (errorCode == 0)
        app_out->info("Minimum audio latency enabled on the DEFAULT playback device!\n");
    else
        app_out->info("ERROR: Could not enable low-latency mode. Error code {}.\n", errorCode);

    app_out->info("Checking for updates...");
    AutoUpdater(APP_VERSION).Update();

    if (errorCode == 0) 
        WaitForAnyKey("\nPress any key to disable and exit . . .");
    else
        WaitForAnyKey("\nPress any key to exit . . .");

    return errorCode;
}
