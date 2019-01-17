@echo off

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set InstallDir=%%i
)

call "%InstallDir%\VC\Auxiliary\Build\vcvars64.bat"
cd winbuild && nmake /f Makefile.vc mode=static VC=15 MACHINE=x64 DEBUG=yes
