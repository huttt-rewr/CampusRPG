#ifndef SPECIALITEM_H
#define SPECIALITEM_H

#include "Item.h"
#include <memory>

enum class SpecialEffect {
    ExpCard,
    WeaponScroll,
    ArmorScroll,
    HpPotion,
    MpPotion,
    RagePotion,
    WeakPotion
};

class SpecialItem : public Item {
private:
    SpecialEffect effect;
    int value;
    int duration;

public:
    SpecialItem(const std::string& name, int price, const std::string& desc,
                SpecialEffect effect, int value, int duration = 0);

    std::string use(Character& c) override;
    std::string useInBattle(Character& c, Enemy* enemy) override;
    std::string getInfo() const override;
    std::string getSaveId() const override;
    SpecialEffect getEffect() const;
    int getValue() const;
    int getDuration() const;
};

std::shared_ptr<Item> createSpecialItemById(const std::string& id);

#endif
