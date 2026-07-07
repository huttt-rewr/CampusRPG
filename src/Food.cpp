#include "Food.h"
#include "Character.h"
#include <sstream>

Food::Food(const std::string& n, int p, const std::string& d, int h)
    : Item(n, "食物", p, d), healAmount(h) {}

std::string Food::use(Character& c) {
    c.heal(healAmount);
    std::ostringstream oss;
    oss << "恢复了 " << healAmount << " 点生命值!";
    return oss.str();
}

std::string Food::getInfo() const {
    std::ostringstream oss;
    oss << Item::getInfo() << " [恢复" << healAmount << "HP]";
    return oss.str();
}
