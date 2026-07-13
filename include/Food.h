// Food.h
// 食物类：派生自物品基类，只能非战斗使用，回复固定生命值。
#ifndef FOOD_H
#define FOOD_H

#include "Item.h"

class Food : public Item {
private:
    int healAmount_;

public:
    Food(std::string name, int price, std::string description, int healAmount);
    ~Food() override = default;

    int getHealAmount() const;
    bool canUseInBattle() const override;
    bool use(Character& player, bool inBattle) override;
    std::string effectText() const override;
    std::shared_ptr<Item> clone() const override;
    std::string saveLine() const override;
};

#endif
