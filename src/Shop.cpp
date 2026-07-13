// Shop.cpp
// 商店类实现：查看商品、购买商品、出售背包物品，并自动结算金币。
#include "Shop.h"
#include "Character.h"
#include "Item.h"
#include <iostream>

Shop::Shop() = default;

void Shop::addGoods(const std::shared_ptr<Item>& item) {
    if (item) goods_.push_back(item);
}

void Shop::showGoods() const {
    std::cout << "================ 商品列表 ================\n";
    for (int i = 0; i < static_cast<int>(goods_.size()); ++i) {
        const auto& item = goods_[i];
        std::cout << i + 1 << ". " << item->getName()
                  << " | 类型：" << item->getType()
                  << " | 价格：" << item->getPrice()
                  << " | 描述：" << item->getDescription()
                  << " | 效果：" << item->effectText() << "\n";
    }
    std::cout << "==========================================\n";
}

bool Shop::buy(Character& player, int index) const {
    if (index < 0 || index >= static_cast<int>(goods_.size())) return false;
    const auto& item = goods_[index];
    if (player.isInventoryFull()) {
        std::cout << "背包已满，购买失败。\n";
        return false;
    }
    if (!player.spendGold(item->getPrice())) {
        std::cout << "金币不足，购买失败。\n";
        return false;
    }
    player.addItem(item->clone());
    std::cout << "购买成功：" << item->getName()
              << "，剩余金币 " << player.getGold() << "。\n";
    return true;
}

bool Shop::sell(Character& player, int inventoryIndex) const {
    auto item = player.getItem(inventoryIndex);
    if (!item) return false;
    int sellPrice = item->getPrice() / 2;
    player.removeItem(inventoryIndex);
    player.addGold(sellPrice);
    std::cout << "出售成功：" << item->getName()
              << "，获得金币 " << sellPrice
              << "，当前金币 " << player.getGold() << "。\n";
    return true;
}

const std::vector<std::shared_ptr<Item>>& Shop::getGoods() const {
    return goods_;
}

std::map<std::string, std::shared_ptr<Item>> Shop::buildItemFactory() const {
    std::map<std::string, std::shared_ptr<Item>> factory;
    for (const auto& item : goods_) {
        factory[item->getName()] = item;
    }
    return factory;
}
