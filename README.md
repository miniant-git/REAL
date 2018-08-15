# REAL - REduce Audio Latency

## Requirements

* Windows 10 64-bit
* [Microsoft Visual C++ 2017 Redistributable (x64)](https://aka.ms/vs/15/release/VC_redist.x64.exe) 

## Setup

1. Install Windows' in-box HDAudio driver:
    1. Start **Device Manager**.
    2. Under **Sound, video and game controllers**, double click on the device that corresponds to your speakers.
    3. In the next window, go to the **Driver** tab.
    4. Select **Update driver** -> **Browse my computer for driver software** -> **Let me pick from a list of available drivers on my computer**.
    5. Select **High Definition Audio Device** and click **Next**.
    6. If a window titled "Update Driver warning" appears, click **Yes**.
    7. Select **Close**.
    8. If asked to reboot the system, select **Yes** to reboot.
2. Download and install the [latest version](https://github.com/miniant-git/REAL/releases/latest) of **REAL**. If using the provided installer:
    1. Double-click on the downloaded file.
    2. Select **More info** -> **Run anyway**.
    3. Follow the installation steps. By default the executable will be installed in `C:\Program Files\miniant\REAL\`. 
3. Launch `REAL.exe`. The latency reduction is in effect as long as the application is open.

## Building

1. Make sure **Microsoft Visual Studio 2017** is installed and updated.
2. Clone the repository: `git clone https://github.com/miniant-git/REAL.git miniant-real`
3. Open `miniant-real.sln` solution and install any missing Visual Studio components.
4. Download `curl` sources by launching `setup.bat` from `/real-app/deps/curl/`.
5. Right-click on the `miniant-real` project in the **Solution Explorer** and select **Build**.
