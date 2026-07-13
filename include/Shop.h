// Shop.h
// 商店类定义：保存商品列表，提供查看、购买和出售接口。
#ifndef SHOP_H
#define SHOP_H

#include <map>
#include <memory>
#include <string>
#include <vector>

class Character;
class Item;

class Shop {
private:
    std::vector<std::shared_ptr<Item>> goods_;

public:
    Shop();
    void addGoods(const std::shared_ptr<Item>& item);
    void showGoods() const;
    bool buy(Character& player, int index) const;
    bool sell(Character& player, int inventoryIndex) const;
    const std::vector<std::shared_ptr<Item>>& getGoods() const;
    std::map<std::string, std::shared_ptr<Item>> buildItemFactory() const;
};

#endif
