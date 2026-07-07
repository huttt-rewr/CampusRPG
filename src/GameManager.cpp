#include "GameManager.h"
#include "Food.h"
#include "Medicine.h"
#include "Equipment.h"
#include <sstream>
#include <cstdlib>
#include <ctime>

GameManager::GameManager() {
    srand((unsigned)time(nullptr));
    map = std::make_unique<GameMap>();
    initEnemies();
    initTasks();
    initShop();
}

GameManager::~GameManager() = default;

void GameManager::initPlayer(const std::string& name) {
    player = std::make_unique<Character>(name);
}

void GameManager::initEnemies() {
    enemies.push_back(std::make_unique<NormalMonster>(
        "史莱姆", 30, 5, 2, 20, 10));
    enemies.push_back(std::make_unique<NormalMonster>(
        "哥布林", 50, 8, 3, 35, 20));
    enemies.push_back(std::make_unique<EliteMonster>(
        "暗影骑士", 80, 15, 8, 60, 40, 0.3, 5));
    enemies.push_back(std::make_unique<EliteMonster>(
        "巨石守卫", 100, 12, 12, 70, 50, 0.2, 10));
    enemies.push_back(std::make_unique<Boss>(
        "黑龙王", 200, 20, 10, 200, 150, "龙息术", 35));
}

void GameManager::initTasks() {
    tasks.push_back(std::make_unique<Task>(
        "初出茅庐", "击败史莱姆证明你的实力!",
        "击败3只史莱姆", 50, 30, 3));
    tasks.push_back(std::make_unique<Task>(
        "哥布林杀手", "击败哥布林保护村庄!",
        "击败5只哥布林", 100, 60, 5));
    tasks.push_back(std::make_unique<Task>(
        "屠龙勇士", "击败黑龙王拯救校园!",
        "击败黑龙王", 500, 300, 1));
}

void GameManager::initShop() {
    shop = std::make_unique<Shop>("校园商店");
    shop->addItem(std::make_shared<Food>("面包", 15, "恢复少量HP", 25));
    shop->addItem(std::make_shared<Food>("烤鸡", 35, "恢复较多HP", 60));
    shop->addItem(std::make_shared<Medicine>("力量药水", 50, "提升攻击力", "attack", 8, 3));
    shop->addItem(std::make_shared<Medicine>("防御药水", 45, "提升防御力", "defense", 6, 3));
    shop->addItem(std::make_shared<Medicine>("急救包", 40, "立即恢复HP", "heal", 100, 1));
    shop->addItem(std::make_shared<Equipment>("铁剑", 80, "普通铁剑", 5, 0));
    shop->addItem(std::make_shared<Equipment>("皮甲", 70, "轻便皮甲", 0, 4));
    shop->addItem(std::make_shared<Equipment>("勇者之剑", 200, "传说中的宝剑", 15, 5));
}

bool GameManager::doBattle(int enemyIndex) {
    if (enemyIndex < 0 || enemyIndex >= (int)enemies.size()) return false;
    auto& enemy = enemies[enemyIndex];
    battleLog.clear();

    while (player->isAlive() && enemy->isAlive()) {
        // Player turn
        int pdmg = player->dealDamage();
        enemy->takeDamage(pdmg);
        std::ostringstream oss;
        oss << player->getName() << " 造成 " << pdmg << " 伤害 | ";
        // Enemy HP bar
        int ehpPct = enemy->getHp() * 100 / enemy->getMaxHp();
        oss << enemy->getName() << " HP: " << enemy->getHp() << "/" << enemy->getMaxHp()
            << " (" << ehpPct << "%)";
        battleLog += oss.str() + "\n";

        if (!enemy->isAlive()) break;

        // Enemy turn
        int edmg = enemy->attackPlayer();
        if (edmg < 0) {
            battleLog += enemy->getName() + " 逃跑了!\n";
            break;
        }
        // Check for Boss skill
        if (auto* boss = dynamic_cast<Boss*>(enemy.get())) {
            if (edmg > enemy->getAttack() + 10) {
                battleLog += boss->getName() + " 使用 " + boss->getSkillName() + "!\n";
            }
            boss->checkPhase();
            if (boss->getPhase() == 2) {
                battleLog += "黑龙王进入第二阶段!\n";
            }
        }
        // Check for Elite crit
        if (auto* elite = dynamic_cast<EliteMonster*>(enemy.get())) {
            if (elite->isCrit()) {
                battleLog += "暴击! ";
            }
        }
        player->takeDamage(edmg);
        battleLog += enemy->getName() + " 造成 " + std::to_string(edmg) + " 伤害 | ";
        battleLog += player->getName() + " HP: " + std::to_string(player->getHp())
                  + "/" + std::to_string(player->getMaxHp()) + "\n";
    }

    if (player->isAlive()) {
        int er = enemy->getExpReward();
        int gr = enemy->getGoldReward();
        std::string expMsg = player->gainExp(er);
        player->addGold(gr);
        battleLog += "\n=== 胜利! ===\n";
        battleLog += expMsg + " | +" + std::to_string(gr) + "G\n";

        // Update task progress
        for (auto& t : tasks) {
            if (t->isAccepted() && !t->isCompleted()) {
                if (t->getName().find("初出茅庐") != std::string::npos &&
                    enemy->getName().find("史莱姆") != std::string::npos) {
                    t->addProgress(1);
                }
                if (t->getName().find("哥布林杀手") != std::string::npos &&
                    enemy->getName().find("哥布林") != std::string::npos) {
                    t->addProgress(1);
                }
                if (t->getName().find("屠龙勇士") != std::string::npos &&
                    enemy->getName().find("黑龙王") != std::string::npos) {
                    t->addProgress(1);
                }
            }
        }
        return true;
    } else {
        battleLog += "\n=== 战败... ===\n失去了部分金币\n";
        player->addGold(-player->getGold() / 5);
        return false;
    }
}

void GameManager::checkTaskProgress() {
    // Tasks are updated during battle
}

std::string GameManager::moveTo(int locIndex) {
    if (!map || !player) return "系统错误";
    return map->moveTo(locIndex);
}

std::string GameManager::rest() {
    if (!map || !player) return "系统错误";
    std::string result = map->rest();
    if (result == "rest_ok") {
        player->heal(player->getMaxHp());
        return "你在" + map->getCurrentLocationName() + "休息了一会儿。\n生命值完全恢复!";
    }
    return result;
}

int GameManager::checkEncounter() {
    if (!map) return -1;
    return map->getRandomEncounter();
}
