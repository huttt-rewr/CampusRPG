@echo off
chcp 65001 > nul
setlocal
set "APP_DIR=%~dp0"
set "PATH=C:\Qt\5.15.2\mingw81_64\bin;D:\mingw64\bin;%PATH%"
cd /d "%APP_DIR%"
qmake CampusRPG.pro -o Makefile
if errorlevel 1 goto fail
mingw32-make -j2
if errorlevel 1 goto fail
echo 构建成功：%APP_DIR%release\CampusRPG_GUI.exe
pause
exit /b 0
:fail
echo 构建失败。
pause
exit /b 1
