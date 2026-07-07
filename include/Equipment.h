#ifndef EQUIPMENT_H
#define EQUIPMENT_H
#include "Item.h"

class Equipment : public Item {
    int attackBonus;
    int defenseBonus;
public:
    Equipment(const std::string& name, int price, const std::string& desc,
              int atkBonus, int defBonus);
    std::string use(Character& c) override;
    std::string getInfo() const override;
};

#endif
