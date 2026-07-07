#ifndef MEDICINE_H
#define MEDICINE_H
#include "Item.h"

class Medicine : public Item {
    std::string buffType;
    int buffValue;
    int duration;
public:
    Medicine(const std::string& name, int price, const std::string& desc,
             const std::string& buffType, int buffValue, int duration);
    std::string use(Character& c) override;
    std::string getInfo() const override;
};

#endif
