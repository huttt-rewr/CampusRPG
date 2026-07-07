#ifndef FOOD_H
#define FOOD_H
#include "Item.h"

class Food : public Item {
    int healAmount;
public:
    Food(const std::string& name, int price, const std::string& desc, int heal);
    std::string use(Character& c) override;
    std::string getInfo() const override;
};

#endif
