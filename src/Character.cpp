// Character.cpp
// 角色类实现：负责角色属性封装、背包维护、金币经验和自动升级。
#include "Character.h"
#include "Item.h"
#include <algorithm>
#include <iostream>
#include <sstream>

Character::Character()
    : Character("未命名同学") {}

Character::Character(const std::string& name)
    : name_(name), level_(1), currentHp_(100), maxHp_(100), exp_(0),
      expToLevel_(100), gold_(100), attack_(18), defense_(6) {}

const std::string& Character::getName() const { return name_; }
int Character::getLevel() const { return level_; }
int Character::getCurrentHp() const { return currentHp_; }
int Character::getMaxHp() const { return maxHp_; }
int Character::getExp() const { return exp_; }
int Character::getExpToLevel() const { return expToLevel_; }
int Character::getGold() const { return gold_; }
int Character::getAttack() const { return attack_; }
int Character::getDefense() const { return defense_; }
bool Character::isAlive() const { return currentHp_ > 0; }
bool Character::isInventoryFull() const { return static_cast<int>(inventory_.size()) >= kInventoryLimit; }
int Character::inventorySize() const { return static_cast<int>(inventory_.size()); }

void Character::setName(const std::string& name) {
    name_ = name.empty() ? "未命名同学" : name;
}

void Character::setFullData(const std::string& name, int level, int currentHp, int maxHp,
                            int exp, int expToLevel, int gold, int attack, int defense) {
    name_ = name.empty() ? "未命名同学" : name;
    level_ = std::max(1, level);
    maxHp_ = std::max(1, maxHp);
    currentHp_ = std::clamp(currentHp, 0, maxHp_);
    exp_ = std::max(0, exp);
    expToLevel_ = std::max(1, expToLevel);
    gold_ = std::max(0, gold);
    attack_ = std::max(1, attack);
    defense_ = std::max(0, defense);
}

void Character::heal(int amount) {
    if (amount <= 0) return;
    currentHp_ = std::min(maxHp_, currentHp_ + amount);
}

void Character::takeDamage(int amount) {
    if (amount <= 0) return;
    currentHp_ = std::max(0, currentHp_ - amount);
}

void Character::addExp(int amount) {
    if (amount <= 0) return;
    exp_ += amount;
    std::cout << "获得经验：" << amount << "\n";
    while (exp_ >= expToLevel_) {
        exp_ -= expToLevel_;
        ++level_;
        expToLevel_ = static_cast<int>(expToLevel_ * 1.35) + 30;
        maxHp_ += 20;
        attack_ += 5;
        defense_ += 3;
        currentHp_ = maxHp_;
        std::cout << "\n[升级提示] 等级提升到 Lv." << level_
                  << "，生命上限 " << maxHp_
                  << "，攻击力 " << attack_
                  << "，防御力 " << defense_
                  << "，生命值已回满。\n";
    }
}

bool Character::spendGold(int amount) {
    if (amount < 0 || gold_ < amount) return false;
    gold_ -= amount;
    return true;
}

void Character::addGold(int amount) {
    if (amount > 0) gold_ += amount;
}

void Character::addAttack(int amount) {
    attack_ = std::max(1, attack_ + amount);
}

void Character::addDefense(int amount) {
    defense_ = std::max(0, defense_ + amount);
}

bool Character::addItem(const std::shared_ptr<Item>& item) {
    if (!item || isInventoryFull()) return false;
    inventory_.push_back(item);
    return true;
}

std::shared_ptr<Item> Character::getItem(int index) const {
    if (index < 0 || index >= static_cast<int>(inventory_.size())) return nullptr;
    return inventory_[index];
}

std::shared_ptr<Item> Character::removeItem(int index) {
    if (index < 0 || index >= static_cast<int>(inventory_.size())) return nullptr;
    auto item = inventory_[index];
    inventory_.erase(inventory_.begin() + index);
    return item;
}

const std::vector<std::shared_ptr<Item>>& Character::getInventory() const {
    return inventory_;
}

std::string Character::info() const {
    std::ostringstream out;
    out << "================ 角色信息 ================\n";
    out << "姓名：" << name_ << "\n";
    out << "等级：Lv." << level_ << "\n";
    out << "生命：" << currentHp_ << "/" << maxHp_ << "\n";
    out << "经验：" << exp_ << "/" << expToLevel_ << "\n";
    out << "金币：" << gold_ << "\n";
    out << "攻击力：" << attack_ << "\n";
    out << "防御力：" << defense_ << "\n";
    out << "背包：" << inventory_.size() << "/" << kInventoryLimit << "\n";
    out << "==========================================\n";
    return out.str();
}
