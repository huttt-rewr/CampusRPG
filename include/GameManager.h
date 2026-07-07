#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <memory>
#include <vector>
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

    GameManager();
    ~GameManager();

    void initPlayer(const std::string& name);
    void initEnemies();
    void initTasks();
    void initShop();

    // Map
    std::string moveTo(int locIndex);
    std::string rest();
    int checkEncounter();

    // Battle
    bool doBattle(int enemyIndex);

    // Tasks
    void checkTaskProgress();
};

#endif
