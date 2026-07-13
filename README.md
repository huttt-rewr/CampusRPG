# 校园RPG冒险游戏系统

2026 年 C++ 程序设计课程设计大作业，主题为《校园RPG冒险游戏系统》。当前版本为 **Qt Widgets 图形窗口版**，启动后直接显示角色、背包、商店、任务、战斗等标签页。

## 功能清单

- Qt 窗口界面：角色、背包、商店、任务、战斗五个页面。
- 角色管理：新建角色、查看属性、升级成长、保存/读取存档。
- 背包管理：查看、使用、删除物品，容量限制 20。
- 物品体系：食物、药品、装备三类物品，不同效果。
- 商店系统：查看商品、购买商品、出售背包物品，自动结算金币。
- 任务系统：未接、已接未完成、已完成未领奖、已领奖四种状态。
- 战斗系统：普通敌人、精英敌人、BOSS 敌人，回合制自动展示战斗日志。
- 等级成长：经验累计、跨级升级、属性成长、升级后生命回满。
- 存档：保存为 `release/campus_rpg_qt_save.ini`。

## 文件结构

```text
src/MainWindow.h      Qt 主窗口类定义
src/MainWindow.cpp    Qt 主窗口、界面和玩法逻辑实现
src/main.cpp          Qt 程序入口

include/              课程设计类定义：角色、物品、敌人、任务、商店、管理器
src/Character.cpp     角色类实现
src/Item.cpp          物品基类实现
src/Food.cpp          食物类实现
src/Medicine.cpp      药品类实现
src/Equipment.cpp     装备类实现
src/Enemy.cpp         敌人类实现
src/Task.cpp          任务类实现
src/Shop.cpp          商店类实现
src/GameManager.cpp   控制台版逻辑保留作课程设计类结构参考

CampusRPG.pro         qmake 构建文件
CMakeLists.txt        CMake 构建文件
BuildGame.cmd         Windows 一键构建脚本
RunGame.cmd           Windows 一键启动脚本
```

## 编译运行

推荐 Qt 5.15.2 + MinGW。Windows 下双击：

```bat
BuildGame.cmd
RunGame.cmd
```

或手动执行：

```bat
qmake CampusRPG.pro -o Makefile
mingw32-make -j2
release\CampusRPG_GUI.exe
```

## STL 使用说明

- `QVector`：Qt 窗口主逻辑中保存背包、商品、任务列表，适合按顺序展示到列表控件。
- `QString` / `QStringList`：Qt 界面文本、存档字段、菜单展示文本。
- `std::vector`：保留在课程设计类 `Character`、`Shop`、`GameManager` 中，用于背包、商品列表、任务列表、敌人模板列表。
- `std::map`：保留在 `GameManager` 中作为物品工厂，通过物品名创建奖励和掉落物品。
- `std::shared_ptr` / `std::unique_ptr`：用于物品和敌人基类指针，体现对象生命周期管理。

## 面向对象体现

- 封装：角色、物品、敌人、任务、商店等类属性均为 `private` 或 `protected`。
- 继承体系一：`Item` 为物品基类，派生 `Food`、`Medicine`、`Equipment`。
- 继承体系二：`Enemy` 为敌人基类，派生 `NormalEnemy`、`EliteEnemy`、`BossEnemy`。
- 多态：`Item` 和 `Enemy` 定义纯虚函数，通过基类指针调用子类重写逻辑。
- 虚析构函数：`Item` 和 `Enemy` 均定义虚析构函数。
- 六个核心类：`Character`、`Item`、`Task`、`Enemy`、`Shop`、`GameManager` 均已实现。
