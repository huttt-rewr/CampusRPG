#include "Shop.h"
#include "Character.h"
#include <sstream>

Shop::Shop(const std::string& n) : name(n) {}

std::vector<std::shared_ptr<Item>>& Shop::getItems() { return items; }

std::string Shop::buyItem(int index, Character& c) {
    if (index < 0 || index >= (int)items.size()) return "无效商品";
    auto& item = items[index];
    if (!c.spendGold(item->getPrice()))
        return "金币不足! 需要 " + std::to_string(item->getPrice()) + "G";
    std::string n = item->getName();
    c.addItem(item);
    return "购买了 " + n + "!";
}

std::string Shop::sellItem(int index, Character& c) {
    auto& inv = c.getInventory();
    if (index < 0 || index >= (int)inv.size()) return "无效物品";
    int price = inv[index]->getPrice() / 2;
    std::string n = inv[index]->getName();
    c.addGold(price);
    c.removeItem(index);
    std::ostringstream oss;
    oss << "出售 " << n << " 获得 " << price << "G";
    return oss.str();
}

void Shop::addItem(std::shared_ptr<Item> item) { items.push_back(item); }
std::string Shop::getName() const { return name; }
