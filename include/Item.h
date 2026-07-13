// Item.h
// 物品继承体系基类：定义纯虚函数，食物、药品、装备通过多态实现不同使用逻辑。
#ifndef ITEM_H
#define ITEM_H

#include <memory>
#include <string>

class Character;

class Item {
protected:
    std::string name_;
    std::string type_;
    int price_;
    std::string description_;

public:
    Item(std::string name, std::string type, int price, std::string description);
    virtual ~Item() = default;

    const std::string& getName() const;
    const std::string& getType() const;
    int getPrice() const;
    const std::string& getDescription() const;

    virtual bool canUseInBattle() const = 0;
    virtual bool use(Character& player, bool inBattle) = 0;
    virtual std::string effectText() const = 0;
    virtual std::shared_ptr<Item> clone() const = 0;
    virtual std::string saveLine() const = 0;
};

#endif
