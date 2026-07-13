// GameManager.cpp
// 游戏管理类实现：负责多级菜单、合法输入、文本存档、任务检查、商店和战斗流程。
#include "GameManager.h"
#include "Equipment.h"
#include "Food.h"
#include "Medicine.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

namespace {

std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream ss(text);
    std::string part;
    while (std::getline(ss, part, delimiter)) parts.push_back(part);
    return parts;
}

TaskStatus statusFromInt(int value) {
    switch (value) {
        case 0: return TaskStatus::NotAccepted;
        case 1: return TaskStatus::Accepted;
        case 2: return TaskStatus::Completed;
        case 3: return TaskStatus::Rewarded;
        default: return TaskStatus::NotAccepted;
    }
}

}  // namespace

GameManager::GameManager()
    : player_(), hasPlayer_(false), saveFile_("campus_rpg_save.txt") {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    initializeData();
}

void GameManager::initializeData() {
    shop_.addGoods(std::make_shared<Food>("校园食堂盒饭", 30, "食堂阿姨手不抖的一餐", 30));
    shop_.addGoods(std::make_shared<Medicine>("校医院退烧药", 60, "校医院常备药，战斗中也能救急", 70));
    shop_.addGoods(std::make_shared<Equipment>("图书馆占座笔", 120, "传说能抢到第一排座位的笔", 10, 4));

    itemFactory_ = shop_.buildItemFactory();

    tasks_.push_back(Task(1, "赶走占座狗", "帮同学夺回图书馆座位。",
                          TaskConditionType::DefeatEnemy, "校园占座狗", 2,
                          80, 50, "校园食堂盒饭"));
    tasks_.push_back(Task(2, "收集应急药", "为班级药箱准备校医院退烧药。",
                          TaskConditionType::CollectItem, "校医院退烧药", 1,
                          60, 40, ""));
    tasks_.push_back(Task(3, "直面挂科传说", "击败期末挂科神龙，证明复习有效。",
                          TaskConditionType::DefeatEnemy, "期末挂科神龙", 1,
                          250, 200, "图书馆占座笔"));

    enemyTemplates_.push_back(std::make_unique<NormalEnemy>());
    enemyTemplates_.push_back(std::make_unique<EliteEnemy>());
    enemyTemplates_.push_back(std::make_unique<BossEnemy>());
}

void GameManager::run() {
    startupMenu();
    if (hasPlayer_) mainMenu();
}

void GameManager::startupMenu() {
    while (true) {
        std::cout << "\n========== 校园RPG冒险游戏系统 ==========\n";
        std::cout << "1. 新建角色\n";
        std::cout << "2. 读取存档\n";
        std::cout << "0. 退出\n";
        std::cout << "========================================\n";
        int choice = readInt("请选择：", 0, 2);
        if (choice == 0) return;
        if (choice == 1) {
            std::string name = readLine("请输入角色名称：");
            player_ = Character(name);
            hasPlayer_ = true;
            saveGame();
            std::cout << "新建角色成功，初始金币100，生命已回满。\n";
            return;
        }
        if (choice == 2) {
            if (loadGame()) {
                std::cout << "读档成功，欢迎回来，" << player_.getName() << "！\n";
                return;
            }
            std::cout << "存档不存在或损坏，请新建角色。\n";
        }
    }
}

void GameManager::mainMenu() {
    while (true) {
        saveGame();
        std::cout << "\n================ 主菜单 ================\n";
        std::cout << "1. 角色管理\n";
        std::cout << "2. 背包管理\n";
        std::cout << "3. 商店系统\n";
        std::cout << "4. 任务系统\n";
        std::cout << "5. 战斗系统\n";
        std::cout << "6. 课程设计说明\n";
        std::cout << "0. 保存并退出\n";
        std::cout << "========================================\n";
        int choice = readInt("请选择：", 0, 6);
        if (choice == 0) {
            saveGame();
            std::cout << "已自动保存，欢迎下次继续冒险。\n";
            return;
        }
        if (choice == 1) characterMenu();
        else if (choice == 2) inventoryMenu();
        else if (choice == 3) shopMenu();
        else if (choice == 4) taskMenu();
        else if (choice == 5) battleMenu();
        else if (choice == 6) showCourseDesignNotes();
    }
}

void GameManager::characterMenu() {
    while (true) {
        std::cout << "\n-------------- 角色管理 --------------\n";
        std::cout << "1. 查看角色完整信息\n";
        std::cout << "2. 手动保存当前数据\n";
        std::cout << "0. 返回主菜单\n";
        int choice = readInt("请选择：", 0, 2);
        if (choice == 0) return;
        if (choice == 1) std::cout << player_.info();
        if (choice == 2) {
            saveGame();
            std::cout << "保存成功。\n";
        }
        pause();
    }
}

void GameManager::inventoryMenu() {
    while (true) {
        checkTasksAfterInventoryChange();
        std::cout << "\n-------------- 背包管理 --------------\n";
        std::cout << "1. 查看背包\n";
        std::cout << "2. 使用物品\n";
        std::cout << "3. 删除物品\n";
        std::cout << "0. 返回主菜单\n";
        int choice = readInt("请选择：", 0, 3);
        if (choice == 0) return;
        if (choice == 1) {
            const auto& inv = player_.getInventory();
            std::cout << "背包容量：" << inv.size() << "/20\n";
            if (inv.empty()) std::cout << "背包为空。\n";
            for (int i = 0; i < static_cast<int>(inv.size()); ++i) {
                std::cout << i + 1 << ". " << inv[i]->getName()
                          << " | 类型：" << inv[i]->getType()
                          << " | 价格：" << inv[i]->getPrice()
                          << " | 描述：" << inv[i]->getDescription()
                          << " | 效果：" << inv[i]->effectText() << "\n";
            }
            pause();
        } else if (choice == 2) {
            useItemFromInventory(false);
            checkTasksAfterInventoryChange();
            saveGame();
        } else if (choice == 3) {
            if (player_.inventorySize() == 0) {
                std::cout << "背包为空，无法删除。\n";
            } else {
                int index = readInt("请输入要删除的物品编号：", 1, player_.inventorySize()) - 1;
                auto item = player_.removeItem(index);
                std::cout << "已删除物品：" << item->getName() << "。\n";
                checkTasksAfterInventoryChange();
                saveGame();
            }
        }
    }
}

void GameManager::shopMenu() {
    while (true) {
        std::cout << "\n-------------- 商店系统 --------------\n";
        std::cout << "当前金币：" << player_.getGold() << "\n";
        std::cout << "1. 查看商品列表\n";
        std::cout << "2. 购买商品\n";
        std::cout << "3. 出售背包物品\n";
        std::cout << "0. 返回主菜单\n";
        int choice = readInt("请选择：", 0, 3);
        if (choice == 0) return;
        if (choice == 1) {
            shop_.showGoods();
            pause();
        } else if (choice == 2) {
            shop_.showGoods();
            int index = readInt("请输入要购买的商品编号：", 1, static_cast<int>(shop_.getGoods().size())) - 1;
            shop_.buy(player_, index);
            checkTasksAfterInventoryChange();
            saveGame();
        } else if (choice == 3) {
            if (player_.inventorySize() == 0) {
                std::cout << "背包为空，无法出售。\n";
            } else {
                const auto& inv = player_.getInventory();
                for (int i = 0; i < static_cast<int>(inv.size()); ++i) {
                    std::cout << i + 1 << ". " << inv[i]->getName()
                              << " | 原价：" << inv[i]->getPrice()
                              << " | 出售价：" << inv[i]->getPrice() / 2 << "\n";
                }
                int index = readInt("请输入要出售的物品编号：", 1, player_.inventorySize()) - 1;
                shop_.sell(player_, index);
                checkTasksAfterInventoryChange();
                saveGame();
            }
        }
    }
}

void GameManager::taskMenu() {
    while (true) {
        checkTasksAfterInventoryChange();
        std::cout << "\n-------------- 任务系统 --------------\n";
        std::cout << "1. 查看所有任务\n";
        std::cout << "2. 接受任务\n";
        std::cout << "3. 领取奖励\n";
        std::cout << "0. 返回主菜单\n";
        int choice = readInt("请选择：", 0, 3);
        if (choice == 0) return;
        if (choice == 1) {
            for (const auto& task : tasks_) std::cout << task.info();
            pause();
        } else if (choice == 2) {
            int id = readInt("请输入任务ID：", 1, static_cast<int>(tasks_.size()));
            if (tasks_[id - 1].accept()) std::cout << "接受任务成功。\n";
            else std::cout << "该任务不能重复接受。\n";
            checkTasksAfterInventoryChange();
            saveGame();
        } else if (choice == 3) {
            int id = readInt("请输入任务ID：", 1, static_cast<int>(tasks_.size()));
            if (tasks_[id - 1].claim(player_, itemFactory_)) {
                std::cout << "奖励领取成功，经验、金币和奖励物品已自动发放。\n";
            } else {
                std::cout << "该任务尚未完成或奖励已领取。\n";
            }
            saveGame();
        }
    }
}

void GameManager::battleMenu() {
    while (true) {
        std::cout << "\n-------------- 战斗系统 --------------\n";
        for (int i = 0; i < static_cast<int>(enemyTemplates_.size()); ++i) {
            std::cout << i + 1 << ". " << enemyTemplates_[i]->info() << "\n";
        }
        std::cout << "0. 返回主菜单\n";
        int choice = readInt("请选择敌人：", 0, static_cast<int>(enemyTemplates_.size()));
        if (choice == 0) return;
        fightEnemy(enemyTemplates_[choice - 1]->clone());
        saveGame();
        if (!player_.isAlive()) player_.heal(player_.getMaxHp() / 2);
    }
}

void GameManager::showCourseDesignNotes() const {
    std::cout << "\n========== 课程设计要求体现说明 ==========\n";
    std::cout << "STL vector：用于背包、商品列表、任务列表、敌人模板列表，适合顺序菜单展示。\n";
    std::cout << "STL map：用于物品工厂，通过物品名称快速创建任务奖励和存档物品。\n";
    std::cout << "STL string：用于角色名、任务描述、商品信息、文本存档字段。\n";
    std::cout << "封装：角色、物品、敌人、任务、商店属性均为 private/protected，只能通过公共接口修改。\n";
    std::cout << "继承/多态：Item -> Food/Medicine/Equipment，Enemy -> NormalEnemy/EliteEnemy/BossEnemy。\n";
    std::cout << "纯虚函数：Item::use/effectText/clone，Enemy::attackPlayer/kind/clone。\n";
    std::cout << "运行时多态：背包使用 shared_ptr<Item>，战斗使用 unique_ptr<Enemy> 调用虚函数。\n";
    std::cout << "========================================\n";
    pause();
}

int GameManager::readInt(const std::string& prompt, int minValue, int maxValue) const {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        std::stringstream ss(line);
        int value = 0;
        char extra = '\0';
        if (ss >> value && !(ss >> extra) && value >= minValue && value <= maxValue) {
            return value;
        }
        std::cout << "输入无效，请输入 " << minValue << " 到 " << maxValue << " 之间的数字。\n";
    }
}

std::string GameManager::readLine(const std::string& prompt) const {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        if (!line.empty()) return line;
        std::cout << "输入不能为空，请重新输入。\n";
    }
}

void GameManager::pause() const {
    std::cout << "按回车继续...";
    std::string ignored;
    std::getline(std::cin, ignored);
}

void GameManager::checkTasksAfterInventoryChange() {
    for (auto& task : tasks_) task.updateByInventory(player_.getInventory());
}

void GameManager::checkTasksAfterEnemyDefeated(const std::string& enemyName) {
    for (auto& task : tasks_) task.updateByEnemy(enemyName);
    checkTasksAfterInventoryChange();
}

void GameManager::saveGame() const {
    if (!hasPlayer_) return;
    std::ofstream out(saveFile_);
    if (!out) {
        std::cout << "保存失败：无法写入存档文件。\n";
        return;
    }
    out << "CampusRPGSaveV1\n";
    out << "PLAYER|" << player_.getName() << "|" << player_.getLevel() << "|"
        << player_.getCurrentHp() << "|" << player_.getMaxHp() << "|"
        << player_.getExp() << "|" << player_.getExpToLevel() << "|"
        << player_.getGold() << "|" << player_.getAttack() << "|"
        << player_.getDefense() << "\n";

    out << "INVENTORY|" << player_.inventorySize() << "\n";
    for (const auto& item : player_.getInventory()) {
        out << item->saveLine() << "\n";
    }

    out << "TASKS|" << tasks_.size() << "\n";
    for (const auto& task : tasks_) {
        out << task.saveLine() << "\n";
    }
}

bool GameManager::loadGame() {
    std::ifstream in(saveFile_);
    if (!in) return false;
    return parseSaveFile(in);
}

bool GameManager::parseSaveFile(std::istream& in) {
    try {
        std::string line;
        if (!std::getline(in, line) || line != "CampusRPGSaveV1") return false;
        if (!std::getline(in, line)) return false;
        auto playerParts = split(line, '|');
        if (playerParts.size() != 10 || playerParts[0] != "PLAYER") return false;
        player_.setFullData(playerParts[1], std::stoi(playerParts[2]), std::stoi(playerParts[3]),
                            std::stoi(playerParts[4]), std::stoi(playerParts[5]),
                            std::stoi(playerParts[6]), std::stoi(playerParts[7]),
                            std::stoi(playerParts[8]), std::stoi(playerParts[9]));

        if (!std::getline(in, line)) return false;
        auto invHeader = split(line, '|');
        if (invHeader.size() != 2 || invHeader[0] != "INVENTORY") return false;
        int invCount = std::stoi(invHeader[1]);
        for (int i = 0; i < invCount; ++i) {
            if (!std::getline(in, line)) return false;
            auto parts = split(line, '|');
            if (parts.empty()) return false;
            std::shared_ptr<Item> item;
            if (parts[0] == "Food" && parts.size() == 5) {
                item = std::make_shared<Food>(parts[1], std::stoi(parts[2]), parts[3], std::stoi(parts[4]));
            } else if (parts[0] == "Medicine" && parts.size() == 5) {
                item = std::make_shared<Medicine>(parts[1], std::stoi(parts[2]), parts[3], std::stoi(parts[4]));
            } else if (parts[0] == "Equipment" && parts.size() == 7) {
                item = std::make_shared<Equipment>(parts[1], std::stoi(parts[2]), parts[3],
                                                   std::stoi(parts[4]), std::stoi(parts[5]), std::stoi(parts[6]) != 0);
            } else {
                return false;
            }
            player_.addItem(item);
        }

        if (!std::getline(in, line)) return false;
        auto taskHeader = split(line, '|');
        if (taskHeader.size() != 2 || taskHeader[0] != "TASKS") return false;
        int taskCount = std::stoi(taskHeader[1]);
        if (taskCount != static_cast<int>(tasks_.size())) return false;
        for (int i = 0; i < taskCount; ++i) {
            if (!std::getline(in, line)) return false;
            auto parts = split(line, '|');
            if (parts.size() != 3) return false;
            int id = std::stoi(parts[0]);
            if (id < 1 || id > static_cast<int>(tasks_.size())) return false;
            tasks_[id - 1].setStatus(statusFromInt(std::stoi(parts[1])));
            tasks_[id - 1].setCurrentCount(std::stoi(parts[2]));
        }
        hasPlayer_ = true;
        return true;
    } catch (...) {
        return false;
    }
}

std::shared_ptr<Item> GameManager::createItemByName(const std::string& name) const {
    auto it = itemFactory_.find(name);
    if (it == itemFactory_.end()) return nullptr;
    return it->second->clone();
}

void GameManager::useItemFromInventory(bool inBattle) {
    if (player_.inventorySize() == 0) {
        std::cout << "背包为空，无法使用物品。\n";
        return;
    }
    const auto& inv = player_.getInventory();
    for (int i = 0; i < static_cast<int>(inv.size()); ++i) {
        std::cout << i + 1 << ". " << inv[i]->getName()
                  << " | 类型：" << inv[i]->getType()
                  << " | 效果：" << inv[i]->effectText() << "\n";
    }
    int index = readInt("请输入要使用的物品编号：", 1, player_.inventorySize()) - 1;
    auto item = player_.getItem(index);
    if (!item) return;
    if (inBattle && !item->canUseInBattle()) {
        std::cout << "该物品不能在战斗中使用。\n";
        return;
    }
    int beforeHp = player_.getCurrentHp();
    bool ok = item->use(player_, inBattle);
    if (!ok) {
        std::cout << "使用失败：可能是满血、战斗限制或当前状态不允许。\n";
        return;
    }
    std::cout << "使用成功：" << item->getName()
              << "，生命 " << beforeHp << " -> " << player_.getCurrentHp() << "。\n";
    if (item->getType() != "装备") player_.removeItem(index);
    saveGame();
}

void GameManager::playerAttackEnemy(Enemy& enemy) const {
    int damage = std::max(1, player_.getAttack() - enemy.getDefense());
    enemy.takeDamage(damage);
    std::cout << player_.getName() << " 攻击 " << enemy.getName()
              << "，造成 " << damage << " 点伤害。\n";
}

bool GameManager::fightEnemy(std::unique_ptr<Enemy> enemy) {
    int round = 1;
    std::cout << "\n战斗开始！敌人：" << enemy->getName() << "\n";
    while (player_.isAlive() && enemy->isAlive()) {
        std::cout << "\n第 " << round << " 回合\n";
        std::cout << "1. 普通攻击\n";
        std::cout << "2. 使用药品\n";
        int action = readInt("请选择行动：", 1, 2);
        if (action == 2) {
            useItemFromInventory(true);
        } else {
            playerAttackEnemy(*enemy);
        }

        std::cout << "状态：玩家 HP " << player_.getCurrentHp() << "/" << player_.getMaxHp()
                  << " | 敌人 HP " << enemy->getCurrentHp() << "/" << enemy->getMaxHp() << "\n";
        if (!enemy->isAlive()) break;

        int damage = enemy->attackPlayer(player_, round);
        std::cout << enemy->getName() << " 反击，造成 " << damage << " 点伤害。\n";
        std::cout << "状态：玩家 HP " << player_.getCurrentHp() << "/" << player_.getMaxHp()
                  << " | 敌人 HP " << enemy->getCurrentHp() << "/" << enemy->getMaxHp() << "\n";
        ++round;
    }

    if (player_.isAlive()) {
        std::cout << "\n战斗胜利！获得 " << enemy->getRewardExp()
                  << " 经验和 " << enemy->getRewardGold() << " 金币。\n";
        player_.addExp(enemy->getRewardExp());
        player_.addGold(enemy->getRewardGold());
        int roll = std::rand() % 100 + 1;
        if (roll <= enemy->getDropRate()) {
            auto item = createItemByName(enemy->getDropItemName());
            if (item && player_.addItem(item)) {
                std::cout << "掉落物品：" << item->getName() << "，已放入背包。\n";
            } else {
                std::cout << "掉落物品但背包已满，无法获得。\n";
            }
        }
        checkTasksAfterEnemyDefeated(enemy->getName());
        return true;
    }

    std::cout << "\n战斗失败，已退回主菜单。当前存档会保留，请购买药品或升级后再挑战。\n";
    saveGame();
    return false;
}
