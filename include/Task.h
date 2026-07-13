// Task.h
// 任务类定义：支持击败指定敌人、收集指定物品两类完成条件。
#ifndef TASK_H
#define TASK_H

#include <map>
#include <memory>
#include <string>
#include <vector>

class Character;
class Item;

enum class TaskStatus {
    NotAccepted,
    Accepted,
    Completed,
    Rewarded
};

enum class TaskConditionType {
    DefeatEnemy,
    CollectItem
};

class Task {
private:
    int id_;
    std::string name_;
    std::string description_;
    TaskConditionType conditionType_;
    std::string targetName_;
    int targetCount_;
    int currentCount_;
    int rewardExp_;
    int rewardGold_;
    std::string rewardItemName_;
    TaskStatus status_;

public:
    Task(int id, std::string name, std::string description,
         TaskConditionType conditionType, std::string targetName, int targetCount,
         int rewardExp, int rewardGold, std::string rewardItemName);

    int getId() const;
    const std::string& getName() const;
    TaskStatus getStatus() const;
    std::string statusText() const;
    std::string conditionText() const;
    std::string rewardText() const;
    std::string info() const;
    bool accept();
    void setStatus(TaskStatus status);
    void setCurrentCount(int count);
    void updateByEnemy(const std::string& enemyName);
    void updateByInventory(const std::vector<std::shared_ptr<Item>>& inventory);
    bool canClaim() const;
    bool claim(Character& player, const std::map<std::string, std::shared_ptr<Item>>& itemFactory);
    std::string saveLine() const;
};

#endif
