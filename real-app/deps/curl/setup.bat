@echo off

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set InstallDir=%%i
)

if exist curl-src rmdir curl-src /s /q
git clone https://github.com/curl/curl.git curl-src || goto :error
cd curl-src
git checkout tags/curl-7_61_1 || goto :error
call "buildconf.bat"
call "%InstallDir%\VC\Auxiliary\Build\vcvars64.bat"
cd winbuild && nmake /f Makefile.vc mode=static VC=15 MACHINE=x64
echo curl setup has completed successfully.
exit

:error
echo curl setup has failed.
exit 1
