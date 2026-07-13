// Medicine.h
// 药品类：派生自物品基类，战斗和非战斗都能使用，回复较多生命值。
#ifndef MEDICINE_H
#define MEDICINE_H

#include "Item.h"

class Medicine : public Item {
private:
    int healAmount_;

public:
    Medicine(std::string name, int price, std::string description, int healAmount);
    ~Medicine() override = default;

    int getHealAmount() const;
    bool canUseInBattle() const override;
    bool use(Character& player, bool inBattle) override;
    std::string effectText() const override;
    std::shared_ptr<Item> clone() const override;
    std::string saveLine() const override;
};

#endif
