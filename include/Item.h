#ifndef ITEM_H
#define ITEM_H

#include <string>

class Character;

class Item {
protected:
    std::string name;
    std::string type;
    int price;
    std::string description;

public:
    Item(const std::string& name, const std::string& type,
         int price, const std::string& desc);
    virtual ~Item() = default;

    virtual std::string use(Character& character) = 0;
    virtual std::string getInfo() const;

    std::string getName() const;
    std::string getType() const;
    int getPrice() const;
    std::string getDesc() const;
};

#endif
