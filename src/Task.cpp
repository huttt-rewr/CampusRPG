// Task.cpp
// 任务类实现：维护任务状态，自动检查战斗/收集进度并发放奖励。
#include "Task.h"
#include "Character.h"
#include "Item.h"
#include <algorithm>
#include <sstream>

Task::Task(int id, std::string name, std::string description,
           TaskConditionType conditionType, std::string targetName, int targetCount,
           int rewardExp, int rewardGold, std::string rewardItemName)
    : id_(id), name_(std::move(name)), description_(std::move(description)),
      conditionType_(conditionType), targetName_(std::move(targetName)),
      targetCount_(targetCount), currentCount_(0), rewardExp_(rewardExp),
      rewardGold_(rewardGold), rewardItemName_(std::move(rewardItemName)),
      status_(TaskStatus::NotAccepted) {}

int Task::getId() const { return id_; }
const std::string& Task::getName() const { return name_; }
TaskStatus Task::getStatus() const { return status_; }

std::string Task::statusText() const {
    switch (status_) {
        case TaskStatus::NotAccepted: return "未接";
        case TaskStatus::Accepted: return "已接未完成";
        case TaskStatus::Completed: return "已完成未领奖";
        case TaskStatus::Rewarded: return "已领奖";
    }
    return "未知";
}

std::string Task::conditionText() const {
    std::ostringstream out;
    out << (conditionType_ == TaskConditionType::DefeatEnemy ? "击败 " : "收集 ")
        << targetName_ << " " << targetCount_ << " 个/次"
        << "（当前 " << currentCount_ << "/" << targetCount_ << "）";
    return out.str();
}

std::string Task::rewardText() const {
    std::ostringstream out;
    out << rewardExp_ << "经验，" << rewardGold_ << "金币";
    if (!rewardItemName_.empty()) out << "，" << rewardItemName_;
    return out.str();
}

std::string Task::info() const {
    std::ostringstream out;
    out << "[" << statusText() << "] 任务" << id_ << "：" << name_ << "\n"
        << "  描述：" << description_ << "\n"
        << "  条件：" << conditionText() << "\n"
        << "  奖励：" << rewardText() << "\n";
    return out.str();
}

bool Task::accept() {
    if (status_ != TaskStatus::NotAccepted) return false;
    status_ = TaskStatus::Accepted;
    return true;
}

void Task::setStatus(TaskStatus status) {
    status_ = status;
}

void Task::setCurrentCount(int count) {
    currentCount_ = std::clamp(count, 0, targetCount_);
}

void Task::updateByEnemy(const std::string& enemyName) {
    if (status_ != TaskStatus::Accepted || conditionType_ != TaskConditionType::DefeatEnemy) return;
    if (enemyName == targetName_) {
        currentCount_ = std::min(targetCount_, currentCount_ + 1);
        if (currentCount_ >= targetCount_) status_ = TaskStatus::Completed;
    }
}

void Task::updateByInventory(const std::vector<std::shared_ptr<Item>>& inventory) {
    if (status_ != TaskStatus::Accepted || conditionType_ != TaskConditionType::CollectItem) return;
    int count = 0;
    for (const auto& item : inventory) {
        if (item && item->getName() == targetName_) ++count;
    }
    currentCount_ = std::min(targetCount_, count);
    if (currentCount_ >= targetCount_) status_ = TaskStatus::Completed;
}

bool Task::canClaim() const {
    return status_ == TaskStatus::Completed;
}

bool Task::claim(Character& player, const std::map<std::string, std::shared_ptr<Item>>& itemFactory) {
    if (!canClaim()) return false;
    player.addExp(rewardExp_);
    player.addGold(rewardGold_);
    if (!rewardItemName_.empty()) {
        auto it = itemFactory.find(rewardItemName_);
        if (it != itemFactory.end()) player.addItem(it->second->clone());
    }
    status_ = TaskStatus::Rewarded;
    return true;
}

std::string Task::saveLine() const {
    std::ostringstream out;
    out << id_ << "|" << static_cast<int>(status_) << "|" << currentCount_;
    return out.str();
}
