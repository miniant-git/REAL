@echo off

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set InstallDir=%%i
)

git clone https://github.com/curl/curl.git curl-src
call "curl-src/buildconf.bat"
call "%InstallDir%\VC\Auxiliary\Build\vcvars64.bat"
cd curl-src\winbuild & nmake /f Makefile.vc mode=static VC=15 MACHINE=x64
