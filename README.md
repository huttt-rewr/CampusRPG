# 校园RPG冒险游戏系统

2026 年 C++ 程序设计课程设计大作业，主题为《校园RPG冒险游戏系统》。当前版本为标准 C++17 控制台程序，不依赖 Qt 或其他第三方库。

## 功能清单

- 启动菜单：新建角色 / 读取存档 / 退出。
- 角色管理：新建角色、查看完整属性、自动升级、手动保存。
- 背包管理：获得、查看、使用、删除物品，容量限制 20。
- 物品体系：食物、药品、装备三类物品，不同使用规则和效果。
- 商店系统：查看商品、购买商品、出售背包物品，自动结算金币。
- 任务系统：未接、已接未完成、已完成未领奖、已领奖四种状态。
- 任务条件：击败指定数量敌人、收集指定数量物品。
- 战斗系统：普通敌人、精英敌人、BOSS 敌人，回合制战斗，战斗中可用药品。
- 等级成长：经验累计、跨级升级、属性成长、升级后生命回满。
- 文本存档：角色属性、背包、任务状态、金币和经验自动保存到 `campus_rpg_save.txt`。

## 文件结构

```text
include/Character.h    角色类定义
include/Item.h         物品基类定义
include/Food.h         食物类定义
include/Medicine.h     药品类定义
include/Equipment.h    装备类定义
include/Enemy.h        敌人基类及三种敌人定义
include/Task.h         任务类定义
include/Shop.h         商店类定义
include/GameManager.h  游戏管理类定义

src/main.cpp           程序入口
src/Character.cpp      角色类实现
src/Item.cpp           物品基类实现
src/Food.cpp           食物类实现
src/Medicine.cpp       药品类实现
src/Equipment.cpp      装备类实现
src/Enemy.cpp          敌人类实现
src/Task.cpp           任务类实现
src/Shop.cpp           商店类实现
src/GameManager.cpp    游戏流程、菜单、存档和战斗实现

CMakeLists.txt         CMake 构建文件
BuildGame.cmd          Windows 一键构建脚本
RunGame.cmd            Windows 一键启动脚本
```

## 编译运行

Windows 下可直接双击：

```bat
BuildGame.cmd
RunGame.cmd
```

也可以手动编译：

```bat
g++ -std=c++17 -Wall -Wextra -Iinclude src\main.cpp src\Character.cpp src\Item.cpp src\Food.cpp src\Medicine.cpp src\Equipment.cpp src\Enemy.cpp src\Task.cpp src\Shop.cpp src\GameManager.cpp -o release\CampusRPG.exe
```

或使用 CMake：

```bat
cmake -S . -B build
cmake --build build
```

## STL 使用说明

- `std::vector`：用于角色背包、商店商品列表、任务列表、敌人模板列表。理由是这些数据需要按顺序展示菜单、按编号访问，`vector` 简单高效。
- `std::map`：用于物品工厂 `itemFactory_`，根据物品名称快速创建任务奖励、战斗掉落和存档中的物品。
- `std::string`：用于角色名称、任务描述、商品信息、存档文本字段，便于文本菜单和文件读写。
- `std::shared_ptr`：用于背包和商品中的物品对象，便于多处安全持有物品基类指针。
- `std::unique_ptr`：用于敌人战斗实例，确保敌人对象生命周期清晰，战斗结束自动释放。

## 面向对象体现

- 封装：所有类的属性均为 `private` 或 `protected`，外部只能通过公共接口读取或修改。
- 继承体系一：`Item` 为物品基类，派生 `Food`、`Medicine`、`Equipment`。
- 继承体系二：`Enemy` 为敌人基类，派生 `NormalEnemy`、`EliteEnemy`、`BossEnemy`。
- 多态：`Item` 和 `Enemy` 均定义纯虚函数，并通过基类指针调用子类重写逻辑。
- 虚析构函数：`Item` 和 `Enemy` 均定义 `virtual ~...() = default`，避免通过基类指针释放派生对象时出错。
- 六个核心类：`Character`、`Item`、`Task`、`Enemy`、`Shop`、`GameManager` 均已实现。
