// GameManager.h
// 游戏管理类定义：组织菜单、存档读写、战斗、商店、任务和背包流程。
#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "Character.h"
#include "Enemy.h"
#include "Shop.h"
#include "Task.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

class GameManager {
private:
    Character player_;
    bool hasPlayer_;
    Shop shop_;
    std::vector<Task> tasks_;
    std::vector<std::unique_ptr<Enemy>> enemyTemplates_;
    std::map<std::string, std::shared_ptr<Item>> itemFactory_;
    std::string saveFile_;

    void initializeData();
    void startupMenu();
    void mainMenu();
    void characterMenu();
    void inventoryMenu();
    void shopMenu();
    void taskMenu();
    void battleMenu();
    void showCourseDesignNotes() const;

    int readInt(const std::string& prompt, int minValue, int maxValue) const;
    std::string readLine(const std::string& prompt) const;
    void pause() const;
    void checkTasksAfterInventoryChange();
    void checkTasksAfterEnemyDefeated(const std::string& enemyName);
    void saveGame() const;
    bool loadGame();
    bool parseSaveFile(std::istream& in);
    std::shared_ptr<Item> createItemByName(const std::string& name) const;
    void useItemFromInventory(bool inBattle);
    void playerAttackEnemy(Enemy& enemy) const;
    bool fightEnemy(std::unique_ptr<Enemy> enemy);

public:
    GameManager();
    void run();
};

#endif
