// Character.h
// 角色类定义：封装玩家角色的属性、升级、金币、背包和格式化输出接口。
#ifndef CHARACTER_H
#define CHARACTER_H

#include <memory>
#include <string>
#include <vector>

class Item;

class Character {
private:
    std::string name_;
    int level_;
    int currentHp_;
    int maxHp_;
    int exp_;
    int expToLevel_;
    int gold_;
    int attack_;
    int defense_;
    std::vector<std::shared_ptr<Item>> inventory_;
    static const int kInventoryLimit = 20;

public:
    Character();
    explicit Character(const std::string& name);

    const std::string& getName() const;
    int getLevel() const;
    int getCurrentHp() const;
    int getMaxHp() const;
    int getExp() const;
    int getExpToLevel() const;
    int getGold() const;
    int getAttack() const;
    int getDefense() const;
    bool isAlive() const;
    bool isInventoryFull() const;
    int inventorySize() const;

    void setName(const std::string& name);
    void setFullData(const std::string& name, int level, int currentHp, int maxHp,
                     int exp, int expToLevel, int gold, int attack, int defense);
    void heal(int amount);
    void takeDamage(int amount);
    void addExp(int amount);
    bool spendGold(int amount);
    void addGold(int amount);
    void addAttack(int amount);
    void addDefense(int amount);
    bool addItem(const std::shared_ptr<Item>& item);
    std::shared_ptr<Item> getItem(int index) const;
    std::shared_ptr<Item> removeItem(int index);
    const std::vector<std::shared_ptr<Item>>& getInventory() const;

    std::string info() const;
};

#endif
