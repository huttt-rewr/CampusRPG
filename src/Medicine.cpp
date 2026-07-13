// Medicine.cpp
// 药品类实现：战斗和非战斗均可使用，回复较多生命。
#include "Medicine.h"
#include "Character.h"
#include <sstream>

Medicine::Medicine(std::string name, int price, std::string description, int healAmount)
    : Item(std::move(name), "药品", price, std::move(description)), healAmount_(healAmount) {}

int Medicine::getHealAmount() const { return healAmount_; }
bool Medicine::canUseInBattle() const { return true; }

bool Medicine::use(Character& player, bool) {
    if (player.getCurrentHp() >= player.getMaxHp()) return false;
    player.heal(healAmount_);
    return true;
}

std::string Medicine::effectText() const {
    std::ostringstream out;
    out << "战斗/非战斗均可使用，回复 " << healAmount_ << " 点生命";
    return out.str();
}

std::shared_ptr<Item> Medicine::clone() const {
    return std::make_shared<Medicine>(*this);
}

std::string Medicine::saveLine() const {
    std::ostringstream out;
    out << "Medicine|" << name_ << "|" << price_ << "|" << description_ << "|" << healAmount_;
    return out.str();
}
