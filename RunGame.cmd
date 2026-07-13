@echo off
chcp 65001 > nul
setlocal
set "APP_DIR=%~dp0"
set "PATH=C:\Qt\5.15.2\mingw81_64\bin;D:\mingw64\bin;%PATH%"
cd /d "%APP_DIR%"
if not exist "%APP_DIR%release\CampusRPG_GUI.exe" (
  call BuildGame.cmd
)
start "" "%APP_DIR%release\CampusRPG_GUI.exe"
