// Item.cpp
// 物品基类实现：保存所有商品共有字段。
#include "Item.h"

Item::Item(std::string name, std::string type, int price, std::string description)
    : name_(std::move(name)), type_(std::move(type)), price_(price), description_(std::move(description)) {}

const std::string& Item::getName() const { return name_; }
const std::string& Item::getType() const { return type_; }
int Item::getPrice() const { return price_; }
const std::string& Item::getDescription() const { return description_; }
