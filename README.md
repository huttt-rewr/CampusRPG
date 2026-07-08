# CampusRPG

校园RPG冒险游戏系统，C++ 程序设计课程设计项目。

## 项目简介

CampusRPG 是一个基于 C++17 和 Qt 的图形化角色扮演游戏。游戏以校园为地图场景，玩家可以创建角色、探索地点、触发战斗、完成任务、购买和使用物品，并通过存档系统保存游戏进度。

## 主要功能

- 角色系统：等级、生命值、蓝量、经验值、金币、攻击力、防御力
- 背包系统：查看、使用、丢弃物品
- 商店系统：购买/出售经验卡、强化卷、回血药、回蓝药、狂暴药水、虚弱药水
- 城镇服务：仓库、银行、铁匠铺，仅城镇节点可使用
- 战斗系统：回合制战斗、敌人特殊行为、战斗药水、死亡掉落惩罚
- 任务系统：接受任务、更新进度、领取奖励
- 地图系统：校园节点移动、随机遇敌、出城探险
- 存档系统：最多 4 个独立存档槽，可新建、读取、保存、删除

## 运行方式

Windows 下可直接运行：

```bat
RunGame.cmd
```

或运行中文启动脚本：

```bat
启动游戏.bat
```

## 构建方式

项目支持 qmake 构建。推荐使用 Qt 5.15.2 + MinGW：

```bat
qmake CampusRPG.pro -o Makefile
mingw32-make -j2
```

也可以直接运行：

```bat
BuildGame.cmd
```

生成的程序位于：

```text
release/CampusRPG_GUI.exe
```

## 项目结构

```text
include/     头文件
src/         源文件和 Qt 界面代码
CampusRPG.pro
CMakeLists.txt
RunGame.cmd
BuildGame.cmd
```

## 技术栈

- C++17
- Qt Widgets
- STL 容器与智能指针
- qmake / CMake
- Git + GitHub
