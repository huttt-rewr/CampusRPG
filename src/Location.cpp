#include "Location.h"
#include <cstdlib>
#include <sstream>

GameMap::GameMap() : currentLocation(0) {
    initCampusMap();
}

void GameMap::initCampusMap() {
    // index 0: 校门口
    locations.push_back(Location(
        "校门口", "校园的正门，新生报到的地方。矗立着校训石。",
        5, 8, "🏫", false, false));
    locations[0].connections = {1, 2, 4};
    locations[0].enemyPool = {0};  // Only slimes

    // index 1: 教学楼
    locations.push_back(Location(
        "教学楼", "高耸的教学楼，上课和自习的地方。",
        5, 6, "📚", false, false));
    locations[1].connections = {0, 2, 3};
    locations[1].enemyPool = {0, 1};  // Slimes + goblins

    // index 2: 图书馆
    locations.push_back(Location(
        "图书馆", "安静的知识殿堂，学霸的聚集地。",
        7, 7, "📖", false, false));
    locations[2].connections = {0, 1, 3, 5};
    locations[2].enemyPool = {0, 1};  // Slimes + goblins

    // index 3: 食堂
    locations.push_back(Location(
        "食堂", "美食飘香的校园食堂，可以恢复体力。",
        7, 5, "🍜", true, true));  // Rest + Shop
    locations[3].connections = {1, 2, 6};
    locations[3].enemyPool = {0};

    // index 4: 操场
    locations.push_back(Location(
        "操场", "广阔的操场，体育课和训练的地方。",
        3, 8, "🏟️", false, false));
    locations[4].connections = {0, 6, 7};
    locations[4].enemyPool = {1, 2};  // Goblins + Elite

    // index 5: 实验室
    locations.push_back(Location(
        "实验室", "神秘的实验室，进行各种科研项目。",
        9, 7, "🔬", false, false));
    locations[5].connections = {2, 8};
    locations[5].enemyPool = {2, 3};  // Elite

    // index 6: 宿舍区
    locations.push_back(Location(
        "宿舍区", "温馨的学生宿舍，可以休息恢复。",
        3, 5, "🏠", true, false));  // Can rest
    locations[6].connections = {3, 4, 8};
    locations[6].enemyPool = {0, 1};

    // index 7: 行政楼
    locations.push_back(Location(
        "行政楼", "校园的行政中枢。",
        1, 8, "🏛️", false, false));
    locations[7].connections = {4, 8, 9};
    locations[7].enemyPool = {1, 2};

    // index 8: 花园
    locations.push_back(Location(
        "花园", "美丽的花园，隐藏着秘密通道。",
        1, 5, "🌸", false, false));
    locations[8].connections = {5, 6, 7, 9};
    locations[8].enemyPool = {2, 3};  // Elite

    // index 9: 地下迷宫
    locations.push_back(Location(
        "地下迷宫", "校园地下深处，传说中BOSS的巢穴！",
        0, 2, "💀", false, false));
    locations[9].connections = {7, 8};
    locations[9].enemyPool = {3, 4};  // Elite + Boss
}

std::vector<int> GameMap::getReachableLocations() const {
    return locations[currentLocation].connections;
}

std::string GameMap::moveTo(int locIndex) {
    // Check if reachable
    bool reachable = false;
    for (int c : locations[currentLocation].connections) {
        if (c == locIndex) { reachable = true; break; }
    }
    if (!reachable) return "无法直接到达该位置!";

    std::string oldName = locations[currentLocation].name;
    currentLocation = locIndex;
    std::ostringstream oss;
    oss << "从 " << oldName << " 移动到了 " << locations[currentLocation].name
        << "\n" << locations[currentLocation].description;
    return oss.str();
}

Location& GameMap::getCurrentLocation() {
    return locations[currentLocation];
}

std::string GameMap::getCurrentLocationName() const {
    return locations[currentLocation].name;
}

int GameMap::getRandomEncounter() const {
    // 40% chance of random encounter when moving
    if (rand() % 100 >= 40) return -1;

    auto& pool = locations[currentLocation].enemyPool;
    if (pool.empty()) return -1;
    return pool[rand() % pool.size()];
}

std::string GameMap::rest() {
    if (!locations[currentLocation].hasRest) {
        return "这里不能休息! 去食堂或宿舍区吧。";
    }
    return "rest_ok";  // Signal to heal player
}
