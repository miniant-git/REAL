# REAL - REduce Audio Latency

## Requirements

* Windows 10 64-bit
* [Microsoft Visual C++ 2017 Redistributable (x64)](https://aka.ms/vs/15/release/VC_redist.x64.exe) 

## Setup

1. Install Windows' in-box HDAudio driver (recommended):
    1. Start **Device Manager**.
    2. Under **Sound, video and game controllers**, double click on the device that corresponds to your speakers.
    3. In the next window, go to the **Driver** tab.
    4. Select **Update driver** -> **Browse my computer for driver software** -> **Let me pick from a list of available drivers on my computer**.
    5. Select **High Definition Audio Device** and click **Next**.
    6. If a window titled "Update Driver warning" appears, click **Yes**.
    7. Select **Close**.
    8. If asked to reboot the system, select **Yes** to reboot.
    > **Be careful**: the new driver might reset your volume to uncomfortably high levels. 
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

## FAQ

### How does this work?

As described in Mirosoft's [Low Latency Audio FAQ section](https://docs.microsoft.com/en-us/windows-hardware/drivers/audio/low-latency-audio#span-idfaqspanspan-idfaqspanfaq), by default, all applications in Windows 10 use 10ms buffers to render audio. However, if one application requests the usage of small buffers, then the Audio Engine will start transferring audio using that particular buffer size. In that case, all applications that use the same endpoint (device) and mode (either exclusive or shared) will automatically switch to that small buffer size. We make use of this Audio Engine property by starting a rendering stream which requests the minimal buffer size that is supported by the audio driver.

### What are the downsides?

Since the application reduces audio buffer size, the CPU has less time to sleep until the buffer needs to be refilled, which technically results in increased power usage. In addition, small buffer size gives the CPU not much time to refill it and might cause audible cracks and glitches when the CPU is unable to keep up.
