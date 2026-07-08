#include "GameManager.h"
#include "Food.h"
#include "Medicine.h"
#include "Equipment.h"
#include "SpecialItem.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStringList>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

GameManager::GameManager()
    : bankGold(0), currentSaveSlot(-1), battlesSinceRest(0),
      mainlineStage(1), restPointAvailable(false) {
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

void GameManager::resetWorldForNewPlayer(const std::string& name) {
    player = std::make_unique<Character>(name);
    warehouse.clear();
    bankGold = 0;
    battlesSinceRest = 0;
    mainlineStage = 1;
    restPointAvailable = false;
    map = std::make_unique<GameMap>();
    enemies.clear();
    tasks.clear();
    initEnemies();
    initTasks();
    initShop();
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
    shop->addItem(createSpecialItemById("战斗经验卡"));
    shop->addItem(createSpecialItemById("武器强化卷"));
    shop->addItem(createSpecialItemById("铠甲强化卷"));
    shop->addItem(createSpecialItemById("回血药"));
    shop->addItem(createSpecialItemById("回蓝药"));
    shop->addItem(createSpecialItemById("狂暴药水"));
    shop->addItem(createSpecialItemById("虚弱药水"));
    shop->addItem(createSpecialItemById("疲惫药水"));
}

bool GameManager::isTown() const {
    if (!map) return false;
    const auto& loc = map->locations[map->currentLocation];
    return loc.hasShop || loc.hasRest;
}

std::string GameManager::currentLocationType() const {
    return isTown() ? "城镇节点" : "野外节点";
}

std::string GameManager::currentTownName() const {
    if (!map) return "起源之地";
    int idx = map->currentLocation;
    if (idx <= 3) return "起源之地";
    if (idx <= 5) return "黄昏边界";
    if (idx <= 7) return "芒星镇";
    if (idx == 8) return "月辉城";
    return "圣都";
}

int GameManager::currentTownMinLevel() const {
    std::string town = currentTownName();
    if (town == "起源之地") return 0;
    if (town == "黄昏边界") return 10;
    if (town == "芒星镇") return 15;
    if (town == "月辉城") return 20;
    return 25;
}

int GameManager::currentTownMaxLevel() const {
    std::string town = currentTownName();
    if (town == "起源之地") return 10;
    if (town == "黄昏边界") return 15;
    if (town == "芒星镇") return 20;
    if (town == "月辉城") return 25;
    return 30;
}

std::string GameManager::mainlineInfo() const {
    static const char* stages[] = {
        "起源之地 -> 黄昏边界",
        "黄昏边界 -> 芒星镇",
        "芒星镇 -> 月辉城",
        "月辉城 -> 圣都",
        "圣都 -> 进军次元裂隙"
    };
    int idx = std::max(1, std::min(mainlineStage, 5)) - 1;
    return std::string("主线阶段 ") + std::to_string(mainlineStage)
        + "：" + stages[idx] + "。当前为框架进度，后续可扩展为关卡推进。";
}

std::string GameManager::formationInfo() const {
    return "编队框架：前排1人，后排2人。当前版本以主角单人战斗承载核心流程，后续可扩展为3v3技能选人与目标选择。";
}

std::string GameManager::addBattleRewardItem() {
    if (!player) return "";
    if (rand() % 100 >= 35) return "本次战斗没有掉落物品。";
    static const char* drops[] = {"回血药", "回蓝药", "战斗经验卡"};
    auto item = createSpecialItemById(drops[rand() % 3]);
    if (!item) return "";
    std::string name = item->getName();
    if (!player->addItem(item)) {
        return "战斗掉落了 " + name + "，但背包已满，无法拾取。";
    }
    return "战斗掉落：" + name + " 已自动放入背包。";
}

void GameManager::recordBattleFinished() {
    ++battlesSinceRest;
    if (battlesSinceRest >= 3) {
        restPointAvailable = true;
        battlesSinceRest = 0;
    }
}

std::string GameManager::returnToTown() {
    if (!map) return "地图未初始化。";
    if (!restPointAvailable && !isTown()) {
        return "尚未发现休息点。每完成3次战斗后可通过休息点回城。";
    }
    map->currentLocation = 3; // 食堂：当前版本的城镇服务中心
    restPointAvailable = false;
    if (player) {
        player->heal(player->getMaxHp());
        player->restoreMana(player->getMaxMp());
    }
    return "已通过休息点返回城镇，并恢复生命值与蓝量。";
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

std::string GameManager::depositItem(int index) {
    if (!isTown()) return "只有城镇节点可以打开仓库。";
    if (!player) return "角色不存在。";
    auto& inv = player->getInventory();
    if (index < 0 || index >= (int)inv.size()) return "请选择要存入仓库的物品。";
    std::string name = inv[index]->getName();
    warehouse.push_back(inv[index]);
    inv.erase(inv.begin() + index);
    return "已将 " + name + " 存入仓库。";
}

std::string GameManager::withdrawItem(int index) {
    if (!isTown()) return "只有城镇节点可以打开仓库。";
    if (!player) return "角色不存在。";
    if (player->isInventoryFull()) return "背包已满，无法从仓库取出物品。";
    if (index < 0 || index >= (int)warehouse.size()) return "请选择要取出的物品。";
    std::string name = warehouse[index]->getName();
    player->addItem(warehouse[index]);
    warehouse.erase(warehouse.begin() + index);
    return "已从仓库取出 " + name + "。";
}

std::string GameManager::depositGold(int amount) {
    if (!isTown()) return "只有城镇节点可以打开银行。";
    if (!player) return "角色不存在。";
    if (amount <= 0) return "请输入大于0的金币数量。";
    if (!player->spendGold(amount)) return "身上金币不足。";
    bankGold += amount;
    return "已存入 " + std::to_string(amount) + " G。";
}

std::string GameManager::withdrawGold(int amount) {
    if (!isTown()) return "只有城镇节点可以打开银行。";
    if (!player) return "角色不存在。";
    if (amount <= 0) return "请输入大于0的金币数量。";
    if (bankGold < amount) return "银行余额不足。";
    bankGold -= amount;
    player->addGold(amount);
    return "已取出 " + std::to_string(amount) + " G。";
}

std::string GameManager::reinforceWithScroll(int inventoryIndex) {
    if (!isTown()) return "只有城镇节点可以打开铁匠铺。";
    if (!player) return "角色不存在。";
    auto& inv = player->getInventory();
    if (inventoryIndex < 0 || inventoryIndex >= (int)inv.size()) return "请选择强化卷。";
    const std::string name = inv[inventoryIndex]->getName();
    if (name != "武器强化卷" && name != "铠甲强化卷") {
        return "铁匠铺只能使用武器强化卷或铠甲强化卷。";
    }
    std::string result = inv[inventoryIndex]->use(*player);
    inv.erase(inv.begin() + inventoryIndex);
    return result;
}

int GameManager::maxSaveSlots() { return 4; }

std::string GameManager::saveDirPath() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.mkpath("saves");
    return dir.filePath("saves").toStdString();
}

bool GameManager::saveExists(int slot) const {
    if (slot < 1 || slot > maxSaveSlots()) return false;
    QDir dir(QString::fromStdString(saveDirPath()));
    return QFile::exists(dir.filePath(QString("slot%1.ini").arg(slot)));
}

std::string GameManager::saveSlotName(int slot) const {
    if (!saveExists(slot)) return "空存档";
    QDir dir(QString::fromStdString(saveDirPath()));
    QSettings s(dir.filePath(QString("slot%1.ini").arg(slot)), QSettings::IniFormat);
    return s.value("meta/name", QString("存档%1").arg(slot)).toString().toStdString();
}

std::string GameManager::saveGame(int slot) {
    if (!player) return "没有可保存的角色。";
    if (slot < 1 || slot > maxSaveSlots()) return "存档槽无效。";

    QDir dir(QString::fromStdString(saveDirPath()));
    QSettings s(dir.filePath(QString("slot%1.ini").arg(slot)), QSettings::IniFormat);
    s.clear();
    s.setValue("meta/name", QString::fromStdString(player->getName()));
    s.setValue("meta/location", map ? map->currentLocation : 0);
    s.setValue("player/level", player->getLevel());
    s.setValue("player/hp", player->getHp());
    s.setValue("player/maxHp", player->getMaxHp());
    s.setValue("player/mp", player->getMp());
    s.setValue("player/maxMp", player->getMaxMp());
    s.setValue("player/exp", player->getExp());
    s.setValue("player/gold", player->getGold());
    s.setValue("player/attack", player->getAttack());
    s.setValue("player/defense", player->getDefense());
    s.setValue("bank/gold", bankGold);
    s.setValue("progress/battlesSinceRest", battlesSinceRest);
    s.setValue("progress/mainlineStage", mainlineStage);
    s.setValue("progress/restPointAvailable", restPointAvailable);

    QStringList invIds;
    for (const auto& item : player->getInventory()) {
        invIds << QString::fromStdString(item->getSaveId());
    }
    s.setValue("items/inventory", invIds);

    QStringList whIds;
    for (const auto& item : warehouse) {
        whIds << QString::fromStdString(item->getSaveId());
    }
    s.setValue("items/warehouse", whIds);
    s.sync();
    currentSaveSlot = slot;
    return "存档 " + std::to_string(slot) + " 已保存。";
}

std::string GameManager::loadGame(int slot) {
    if (!saveExists(slot)) return "该存档槽为空。";
    QDir dir(QString::fromStdString(saveDirPath()));
    QSettings s(dir.filePath(QString("slot%1.ini").arg(slot)), QSettings::IniFormat);

    std::string name = s.value("meta/name", "勇者").toString().toStdString();
    resetWorldForNewPlayer(name);
    player->setState(name,
        s.value("player/level", 1).toInt(),
        s.value("player/hp", 100).toInt(),
        s.value("player/maxHp", 100).toInt(),
        s.value("player/mp", 60).toInt(),
        s.value("player/maxMp", 60).toInt(),
        s.value("player/exp", 0).toInt(),
        s.value("player/gold", 100).toInt(),
        s.value("player/attack", 12).toInt(),
        s.value("player/defense", 6).toInt());
    bankGold = s.value("bank/gold", 0).toInt();
    battlesSinceRest = s.value("progress/battlesSinceRest", 0).toInt();
    mainlineStage = s.value("progress/mainlineStage", 1).toInt();
    restPointAvailable = s.value("progress/restPointAvailable", false).toBool();
    if (map) {
        map->currentLocation = s.value("meta/location", 0).toInt();
    }

    player->clearInventory();
    QStringList invIds = s.value("items/inventory").toStringList();
    for (const QString& id : invIds) {
        auto item = createSpecialItemById(id.toStdString());
        if (item) player->addItem(item);
    }

    warehouse.clear();
    QStringList whIds = s.value("items/warehouse").toStringList();
    for (const QString& id : whIds) {
        auto item = createSpecialItemById(id.toStdString());
        if (item) warehouse.push_back(item);
    }

    currentSaveSlot = slot;
    return "已读取存档 " + std::to_string(slot) + "。";
}

std::string GameManager::deleteSave(int slot) {
    if (slot < 1 || slot > maxSaveSlots()) return "存档槽无效。";
    QDir dir(QString::fromStdString(saveDirPath()));
    QString path = dir.filePath(QString("slot%1.ini").arg(slot));
    if (!QFile::exists(path)) return "该存档槽已经为空。";
    QFile::remove(path);
    if (currentSaveSlot == slot) currentSaveSlot = -1;
    return "已删除存档 " + std::to_string(slot) + "。";
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
