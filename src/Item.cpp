#include "Item.h"
#include <sstream>

Item::Item(const std::string& n, const std::string& t, int p, const std::string& d)
    : name(n), type(t), price(p), description(d) {}

std::string Item::getInfo() const {
    std::ostringstream oss;
    oss << "[" << type << "] " << name << " - " << description
        << " (" << price << "G)";
    return oss.str();
}

std::string Item::getName() const { return name; }
std::string Item::getType() const { return type; }
int Item::getPrice() const { return price; }
std::string Item::getDesc() const { return description; }
