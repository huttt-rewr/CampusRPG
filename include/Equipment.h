// Equipment.h
// 装备类：派生自物品基类，使用时切换穿戴状态并增减角色攻防。
#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include "Item.h"

class Equipment : public Item {
private:
    int attackBonus_;
    int defenseBonus_;
    bool equipped_;

public:
    Equipment(std::string name, int price, std::string description,
              int attackBonus, int defenseBonus, bool equipped = false);
    ~Equipment() override = default;

    int getAttackBonus() const;
    int getDefenseBonus() const;
    bool isEquipped() const;
    bool canUseInBattle() const override;
    bool use(Character& player, bool inBattle) override;
    std::string effectText() const override;
    std::shared_ptr<Item> clone() const override;
    std::string saveLine() const override;
};

#endif
