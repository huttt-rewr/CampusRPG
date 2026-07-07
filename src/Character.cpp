#include "Character.h"
#include "Item.h"
#include <algorithm>
#include <sstream>

Character::Character()
    : name("Unknown"), level(1), hp(100), maxHp(100),
      exp(0), gold(0), attack(10), defense(5) {
    calcExpToNext();
}

Character::Character(const std::string& name)
    : name(name), level(1), hp(100), maxHp(100),
      exp(0), gold(100), attack(12), defense(6) {
    calcExpToNext();
}

void Character::calcExpToNext() {
    expToNextLevel = level * 100 + (level - 1) * 50;
}

std::string Character::getName() const { return name; }
int Character::getLevel() const { return level; }
int Character::getHp() const { return hp; }
int Character::getMaxHp() const { return maxHp; }
int Character::getExp() const { return exp; }
int Character::getExpToNextLevel() const { return expToNextLevel; }
int Character::getGold() const { return gold; }
int Character::getAttack() const { return attack; }
int Character::getDefense() const { return defense; }

int Character::dealDamage() const {
    return std::max(1, attack + rand() % 5);
}

void Character::takeDamage(int damage) {
    int actual = std::max(1, damage - defense / 3);
    hp = std::max(0, hp - actual);
}

bool Character::isAlive() const { return hp > 0; }

std::string Character::gainExp(int amount) {
    exp += amount;
    std::ostringstream oss;
    oss << "+" << amount << " EXP";
    int levelUps = 0;
    while (exp >= expToNextLevel) {
        exp -= expToNextLevel;
        levelUps++;
        level++;
        attack += 3;
        defense += 2;
        maxHp += 15;
        hp = maxHp;
        calcExpToNext();
    }
    if (levelUps > 0) {
        oss << " | 升级! Lv." << level
            << " (ATK+" << levelUps * 3
            << " DEF+" << levelUps * 2
            << " HP+" << levelUps * 15 << ")";
    }
    return oss.str();
}

std::string Character::levelUp() {
    return gainExp(expToNextLevel - exp);
}

void Character::addGold(int amount) { gold += amount; }
bool Character::spendGold(int amount) {
    if (gold >= amount) { gold -= amount; return true; }
    return false;
}

void Character::addItem(std::shared_ptr<Item> item) {
    inventory.push_back(item);
}

std::string Character::useItem(int index) {
    if (index < 0 || index >= (int)inventory.size()) return "无效物品";
    auto& item = inventory[index];
    std::string result = item->getName() + ": " + item->use(*this);
    inventory.erase(inventory.begin() + index);
    return result;
}

std::string Character::removeItem(int index) {
    if (index < 0 || index >= (int)inventory.size()) return "无效物品";
    std::string n = inventory[index]->getName();
    inventory.erase(inventory.begin() + index);
    return "丢弃了 " + n;
}

std::vector<std::shared_ptr<Item>>& Character::getInventory() { return inventory; }
int Character::inventoryCount() const { return (int)inventory.size(); }

void Character::heal(int amount) { hp = std::min(maxHp, hp + amount); }
void Character::buffAttack(int amount) { attack += amount; }
void Character::buffDefense(int amount) { defense += amount; }
