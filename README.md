# 校园RPG冒险游戏

2026 年 C++ 程序设计课程设计大作业。当前版本为 **Qt Widgets 窗口版校园题材轮回 Roguelike 回合制 RPG**。

## 已实现功能

- 4 个存档位：首页显示「空白存档」或角色名，已创建存档可读取或删除。
- 6 个预设职业：学生、冰法师、圣骑士、祈福者、血战士、魔术师，职业属性和技能按需求写入。
- 轮回主线：第一学期排课、寒假休息/打工、第二学期排课、暑假天使商店、7 层神秘地窟、团灭轮回、最终 BOSS 通关结局。
- 排课系统：一周七天、每天上午/下午，前五天全行动，周末只允许不上课/打工/补习班；显示本周预计盈利和剩余活力。
- 商店系统：天使商店含药品、食品、养成类；恶魔商店含消耗品、装备、恶魔之友，支持出售装备。
- 地窟系统：前 6 层每层 7 房间，第一房间恶魔商店，包含宝箱、普通战斗、精英通道；第 7 层为恶魔商店和最终 BOSS。
- 三对三回合制战斗：按平均等级判断先后手，敌人优先攻击前排，部分敌人可攻击后排，战斗中可用药品/消耗品。
- 编队系统：支持「前排1后排2」和「前排2后排1」，支持调整站位。
- 背包系统：查看、使用、丢弃、装备穿戴；食品不可战斗中使用。
- 任务系统：查看任务、接受任务、自动检测完成、领取奖励。
- 图鉴系统：未遇见敌人显示问号，遇见后显示属性和技能。
- 文本存档：保存到 `release/saves/slot_*.ini`，方便调试查看。

说明：需求中“地窟每层小怪、精英、装备的完整数值表”没有提供具体表格，当前代码按 7 层规则内置了可运行的分层怪物和装备配置。拿到完整表后，只需要替换 `src/MainWindow.cpp` 中 `makeEnemyGroup()` 和 `makeLayerEquipments()` 的配置。

## 文件结构

```text
src/main.cpp           Qt 程序入口
src/MainWindow.h       Qt 主窗口、存档、流程和界面数据结构定义
src/MainWindow.cpp     Qt 界面、轮回、排课、商店、地窟、战斗、任务、存档实现
include/Profession.h   职业抽象基类与技能结构
src/Profession.cpp     六个职业派生类实现

include/Item.h         物品抽象基类
include/Food.h         食品派生类
include/Medicine.h     药品派生类
include/Equipment.h    装备派生类
include/Enemy.h        敌人抽象基类及普通/精英/BOSS派生类
include/Character.h    角色类
include/Task.h         任务类
include/Shop.h         商店类
include/GameManager.h  控制台课程结构保留类

CampusRPG.pro          qmake 构建文件
CMakeLists.txt         CMake 构建文件
BuildGame.cmd          Windows 一键构建
RunGame.cmd            Windows 一键启动
启动游戏.bat            中文启动脚本
构建项目.bat            中文构建脚本
```

## 编译运行

推荐使用 Qt 5.15.2 + MinGW：

```bat
BuildGame.cmd
RunGame.cmd
```

手动编译：

```bat
qmake CampusRPG.pro -o Makefile
mingw32-make -j2
release\CampusRPG_GUI.exe
```

## 面向对象三大特性

- 封装：角色、物品、敌人、任务、职业等数据均通过类和结构统一管理，核心类成员为私有或保护成员，对外通过公共接口访问。
- 继承：`Profession` 派生 6 个职业；`Item` 派生 `Food`、`Medicine`、`Equipment`；`Enemy` 派生普通敌人、精英敌人、BOSS 敌人。
- 多态：`Profession::skills()`、`Profession::baseStats()`、`Item::use()`、`Enemy::attackPlayer()` 等为虚函数或纯虚函数，通过基类指针动态调用派生类逻辑。
- 虚析构：`Profession`、`Item`、`Enemy` 均定义虚析构函数，避免通过基类指针释放时泄漏。

## STL 容器使用说明

- `std::vector<std::unique_ptr<Profession>>`：保存六个职业多态对象，适合顺序遍历并保证对象生命周期安全。
- `std::vector<SkillDef>`：每个职业返回技能表，便于按等级展示和扩展技能。
- `std::unique_ptr`：保存职业基类指针，体现运行时多态和自动释放。
- `std::map`：保留在控制台课程结构类中用于物品工厂映射。
- `QVector`、`QMap`、`QStringList`：Qt 界面层保存角色、背包、任务、房间、图鉴和显示文本，便于和 Qt 控件直接交互。
