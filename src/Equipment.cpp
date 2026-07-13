// Equipment.cpp
// 装备类实现：使用时切换穿戴状态，动态修改角色攻防。
#include "Equipment.h"
#include "Character.h"
#include <sstream>

Equipment::Equipment(std::string name, int price, std::string description,
                     int attackBonus, int defenseBonus, bool equipped)
    : Item(std::move(name), "装备", price, std::move(description)),
      attackBonus_(attackBonus), defenseBonus_(defenseBonus), equipped_(equipped) {}

int Equipment::getAttackBonus() const { return attackBonus_; }
int Equipment::getDefenseBonus() const { return defenseBonus_; }
bool Equipment::isEquipped() const { return equipped_; }
bool Equipment::canUseInBattle() const { return false; }

bool Equipment::use(Character& player, bool inBattle) {
    if (inBattle) return false;
    if (equipped_) {
        player.addAttack(-attackBonus_);
        player.addDefense(-defenseBonus_);
        equipped_ = false;
    } else {
        player.addAttack(attackBonus_);
        player.addDefense(defenseBonus_);
        equipped_ = true;
    }
    return true;
}

std::string Equipment::effectText() const {
    std::ostringstream out;
    out << "切换穿戴状态，攻击+" << attackBonus_ << "，防御+" << defenseBonus_
        << "，当前：" << (equipped_ ? "已穿戴" : "未穿戴");
    return out.str();
}

std::shared_ptr<Item> Equipment::clone() const {
    return std::make_shared<Equipment>(*this);
}

std::string Equipment::saveLine() const {
    std::ostringstream out;
    out << "Equipment|" << name_ << "|" << price_ << "|" << description_
        << "|" << attackBonus_ << "|" << defenseBonus_ << "|" << (equipped_ ? 1 : 0);
    return out.str();
}
