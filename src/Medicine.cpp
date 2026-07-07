#include "Medicine.h"
#include "Character.h"
#include <sstream>

Medicine::Medicine(const std::string& n, int p, const std::string& d,
                   const std::string& bt, int bv, int dur)
    : Item(n, "药品", p, d), buffType(bt), buffValue(bv), duration(dur) {}

std::string Medicine::use(Character& c) {
    std::ostringstream oss;
    if (buffType == "attack") {
        c.buffAttack(buffValue);
        oss << "攻击力+" << buffValue << " (" << duration << "回合)";
    } else if (buffType == "defense") {
        c.buffDefense(buffValue);
        oss << "防御力+" << buffValue << " (" << duration << "回合)";
    } else if (buffType == "heal") {
        c.heal(buffValue);
        oss << "恢复" << buffValue << "HP";
    }
    return oss.str();
}

std::string Medicine::getInfo() const {
    std::ostringstream oss;
    oss << Item::getInfo() << " [" << buffType << "+" << buffValue << "]";
    return oss.str();
}
