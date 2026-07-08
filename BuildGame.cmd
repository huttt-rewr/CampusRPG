@echo off
setlocal
set "APP_DIR=%~dp0"
set "PATH=C:\Qt\5.15.2\mingw81_64\bin;D:\mingw64\bin;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
cd /d "%APP_DIR%"
qmake CampusRPG.pro -o Makefile
if errorlevel 1 goto fail
mingw32-make -j2
if errorlevel 1 goto fail
echo Build complete: %APP_DIR%release\CampusRPG_GUI.exe
pause
exit /b 0
:fail
echo Build failed.
pause
exit /b 1
