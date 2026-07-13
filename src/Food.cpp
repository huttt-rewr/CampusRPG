// Food.cpp
// 食物类实现：非战斗回复固定生命，满血或战斗中使用会失败。
#include "Food.h"
#include "Character.h"
#include <sstream>

Food::Food(std::string name, int price, std::string description, int healAmount)
    : Item(std::move(name), "食物", price, std::move(description)), healAmount_(healAmount) {}

int Food::getHealAmount() const { return healAmount_; }
bool Food::canUseInBattle() const { return false; }

bool Food::use(Character& player, bool inBattle) {
    if (inBattle) return false;
    if (player.getCurrentHp() >= player.getMaxHp()) return false;
    player.heal(healAmount_);
    return true;
}

std::string Food::effectText() const {
    std::ostringstream out;
    out << "非战斗使用，回复 " << healAmount_ << " 点生命";
    return out.str();
}

std::shared_ptr<Item> Food::clone() const {
    return std::make_shared<Food>(*this);
}

std::string Food::saveLine() const {
    std::ostringstream out;
    out << "Food|" << name_ << "|" << price_ << "|" << description_ << "|" << healAmount_;
    return out.str();
}
