@echo off
chcp 65001 > nul
setlocal
set PATH=D:\mingw64\bin;%PATH%
if not exist release mkdir release
g++ -std=c++17 -Wall -Wextra -Iinclude ^
 src\main.cpp src\Character.cpp src\Item.cpp src\Food.cpp src\Medicine.cpp src\Equipment.cpp ^
 src\Enemy.cpp src\Task.cpp src\Shop.cpp src\GameManager.cpp ^
 -o release\CampusRPG.exe
if errorlevel 1 (
  echo 构建失败，请检查 g++ 是否安装并在 PATH 中。
  pause
  exit /b 1
)
echo 构建成功：release\CampusRPG.exe
pause
