#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <memory>
#include <vector>
#include <string>
#include "Character.h"
#include "Enemy.h"
#include "Task.h"
#include "Shop.h"
#include "Location.h"

class GameManager {
public:
    std::unique_ptr<Character> player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Task>> tasks;
    std::unique_ptr<Shop> shop;
    std::unique_ptr<GameMap> map;
    std::string battleLog;
    std::string messageLog;
    std::vector<std::shared_ptr<Item>> warehouse;
    int bankGold;
    int currentSaveSlot;

    GameManager();
    ~GameManager();

    void initPlayer(const std::string& name);
    void initEnemies();
    void initTasks();
    void initShop();
    void resetWorldForNewPlayer(const std::string& name);
    bool isTown() const;
    std::string currentLocationType() const;

    // Map
    std::string moveTo(int locIndex);
    std::string rest();
    int checkEncounter();

    // Battle
    bool doBattle(int enemyIndex);

    // Tasks
    void checkTaskProgress();

    // Storage and bank
    std::string depositItem(int index);
    std::string withdrawItem(int index);
    std::string depositGold(int amount);
    std::string withdrawGold(int amount);
    std::string reinforceWithScroll(int inventoryIndex);

    // Save slots
    static int maxSaveSlots();
    static std::string saveDirPath();
    bool saveExists(int slot) const;
    std::string saveSlotName(int slot) const;
    std::string saveGame(int slot);
    std::string loadGame(int slot);
    std::string deleteSave(int slot);
};

#endif
