@echo off
setlocal
set "APP_DIR=%~dp0"
set "PATH=C:\Qt\5.15.2\mingw81_64\bin;D:\mingw64\bin;%PATH%"
cd /d "%APP_DIR%"
if exist "%APP_DIR%release\CampusRPG_GUI.exe" (
  start "" "%APP_DIR%release\CampusRPG_GUI.exe"
  exit /b 0
)
echo CampusRPG_GUI.exe was not found.
echo Run BuildGame.cmd first, then run this file again.
pause
exit /b 1
