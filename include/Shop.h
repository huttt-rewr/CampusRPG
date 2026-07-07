#ifndef SHOP_H
#define SHOP_H

#include <string>
#include <vector>
#include <memory>
#include "Item.h"

class Character;

class Shop {
private:
    std::string name;
    std::vector<std::shared_ptr<Item>> items;

public:
    explicit Shop(const std::string& name);

    std::vector<std::shared_ptr<Item>>& getItems();
    std::string buyItem(int index, Character& c);
    std::string sellItem(int index, Character& c);
    void addItem(std::shared_ptr<Item> item);
    std::string getName() const;
};

#endif
