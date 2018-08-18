#include "AutoUpdater.h"
#include "MinimumLatencyAudioClient.h"

#include <conio.h>
#include <iostream>

using namespace miniant::WasapiLatency;

constexpr Version APP_VERSION(0, 1, 1);

void WaitForAnyKey(const std::string& message) {
    while (_kbhit())
        _getch();
    
    std::cout << message << std::endl;
    _getch();
}

int main(int argc, char** argv) {
    std::cout << "REAL - REduce Audio Latency " << APP_VERSION.ToString() << ", mini)(ant, 2018"<< std::endl 
        << "Project repository: https://github.com/miniant-git/REAL" << std::endl << std::endl;

    int errorCode = MinimumLatencyAudioClient().Start();
    if (errorCode == 0)
        std::cout << "Minimum audio latency enabled on the DEFAULT playback device!" << std::endl << std::endl;
    else
        std::cout << "ERROR: Could not enable low-latency mode. Error code " << errorCode << "." << std::endl << std::endl;

    std::cout << "Checking for updates..." << std::endl;
    AutoUpdater(APP_VERSION, argv[0]).Update();

    if (errorCode == 0) 
        WaitForAnyKey("\nPress any key to disable and exit . . .");
    else
        WaitForAnyKey("\nPress any key to exit . . .");

    return errorCode;
}
