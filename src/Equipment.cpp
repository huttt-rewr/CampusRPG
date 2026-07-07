#include "Equipment.h"
#include "Character.h"
#include <sstream>

Equipment::Equipment(const std::string& n, int p, const std::string& d,
                     int ab, int db)
    : Item(n, "装备", p, d), attackBonus(ab), defenseBonus(db) {}

std::string Equipment::use(Character& c) {
    std::ostringstream oss;
    if (attackBonus > 0) { c.buffAttack(attackBonus); oss << "ATK+" << attackBonus << " "; }
    if (defenseBonus > 0) { c.buffDefense(defenseBonus); oss << "DEF+" << defenseBonus; }
    oss << "(永久)";
    return oss.str();
}

std::string Equipment::getInfo() const {
    std::ostringstream oss;
    oss << Item::getInfo() << " [ATK+" << attackBonus << " DEF+" << defenseBonus << "]";
    return oss.str();
}
