@echo off
chcp 65001 > nul
setlocal
cd /d "%~dp0"
if not exist release\CampusRPG.exe (
  call BuildGame.cmd
)
release\CampusRPG.exe
pause
