#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include <memory>

class Item;

class Character {
private:
    std::string name;
    int level;
    int hp;
    int maxHp;
    int mp;
    int maxMp;
    int exp;
    int expToNextLevel;
    int gold;
    int attack;
    int defense;
    std::vector<std::shared_ptr<Item>> inventory;

    void calcExpToNext();

public:
    static constexpr int MaxLevel = 30;
    static constexpr int MaxInventorySlots = 20;

    Character();
    explicit Character(const std::string& name);

    // Getters
    std::string getName() const;
    int getLevel() const;
    int getHp() const;
    int getMaxHp() const;
    int getMp() const;
    int getMaxMp() const;
    int getExp() const;
    int getExpToNextLevel() const;
    int getGold() const;
    int getAttack() const;
    int getDefense() const;

    // Combat
    int dealDamage() const;
    void takeDamage(int damage);
    bool isAlive() const;

    // Growth - return strings for UI display
    std::string gainExp(int amount);
    std::string levelUp();
    void addGold(int amount);
    bool spendGold(int amount);
    void setState(const std::string& newName, int newLevel, int newHp, int newMaxHp,
                  int newMp, int newMaxMp, int newExp, int newGold,
                  int newAttack, int newDefense);

    // Inventory
    bool addItem(std::shared_ptr<Item> item);
    bool isInventoryFull() const;
    std::string useItem(int index);
    std::string removeItem(int index);
    std::vector<std::shared_ptr<Item>>& getInventory();
    int inventoryCount() const;
    void clearInventory();

    // Buffs
    void heal(int amount);
    void restoreMana(int amount);
    void buffAttack(int amount);
    void buffDefense(int amount);
};

#endif
