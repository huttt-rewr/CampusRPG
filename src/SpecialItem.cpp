#include "SpecialItem.h"
#include "Character.h"
#include "Enemy.h"
#include "Food.h"
#include "Medicine.h"
#include "Equipment.h"
#include <sstream>

SpecialItem::SpecialItem(const std::string& n, int p, const std::string& d,
                         SpecialEffect e, int v, int dur)
    : Item(n, "道具", p, d), effect(e), value(v), duration(dur) {}

std::string SpecialItem::use(Character& c) {
    std::ostringstream oss;
    switch (effect) {
    case SpecialEffect::ExpCard:
        oss << c.gainExp(value);
        break;
    case SpecialEffect::WeaponScroll:
        c.buffAttack(value);
        oss << "攻击力永久提升 " << value << " 点";
        break;
    case SpecialEffect::ArmorScroll:
        c.buffDefense(value);
        oss << "防御力永久提升 " << value << " 点";
        break;
    case SpecialEffect::HpPotion:
        c.heal(c.getMaxHp() * value / 100);
        oss << "恢复最大生命值的 " << value << "%";
        break;
    case SpecialEffect::MpPotion:
        c.restoreMana(c.getMaxMp() * value / 100);
        oss << "恢复最大蓝量的 " << value << "%";
        break;
    case SpecialEffect::RagePotion:
        c.buffAttack(value);
        oss << "攻击力提升 " << value << " 点，持续 " << duration << " 回合";
        break;
    case SpecialEffect::WeakPotion:
        oss << "虚弱药水只能在战斗中对敌人使用";
        break;
    case SpecialEffect::FatiguePotion:
        oss << "疲惫药水只能在战斗中对敌人使用";
        break;
    }
    return oss.str();
}

std::string SpecialItem::useInBattle(Character& c, Enemy* enemy) {
    if (effect == SpecialEffect::WeakPotion) {
        if (!enemy) return "当前没有可虚弱的敌人";
        enemy->buffAttack(-value);
        std::ostringstream oss;
        oss << enemy->getName() << " 的攻击力降低 " << value << " 点";
        return oss.str();
    }
    if (effect == SpecialEffect::FatiguePotion) {
        if (!enemy) return "当前没有可疲惫的敌人";
        enemy->applyFatigue(duration);
        std::ostringstream oss;
        oss << enemy->getName() << " 陷入疲惫，接下来 " << duration
            << " 回合有50%概率跳过行动";
        return oss.str();
    }
    return use(c);
}

std::string SpecialItem::getInfo() const {
    std::ostringstream oss;
    oss << Item::getInfo();
    switch (effect) {
    case SpecialEffect::ExpCard:
        oss << " [EXP+" << value << "]";
        break;
    case SpecialEffect::WeaponScroll:
        oss << " [永久ATK+" << value << "]";
        break;
    case SpecialEffect::ArmorScroll:
        oss << " [永久DEF+" << value << "]";
        break;
    case SpecialEffect::HpPotion:
        oss << " [HP+" << value << "%]";
        break;
    case SpecialEffect::MpPotion:
        oss << " [MP+" << value << "%]";
        break;
    case SpecialEffect::RagePotion:
        oss << " [ATK+" << value << " / " << duration << "回合]";
        break;
    case SpecialEffect::WeakPotion:
        oss << " [敌ATK-" << value << "]";
        break;
    case SpecialEffect::FatiguePotion:
        oss << " [敌跳过行动50% / " << duration << "回合]";
        break;
    }
    return oss.str();
}

std::string SpecialItem::getSaveId() const { return name; }
SpecialEffect SpecialItem::getEffect() const { return effect; }
int SpecialItem::getValue() const { return value; }
int SpecialItem::getDuration() const { return duration; }

std::shared_ptr<Item> createSpecialItemById(const std::string& id) {
    if (id == "战斗经验卡") {
        return std::make_shared<SpecialItem>("战斗经验卡", 60, "直接提升自己的经验值",
                                             SpecialEffect::ExpCard, 80);
    }
    if (id == "武器强化卷") {
        return std::make_shared<SpecialItem>("武器强化卷", 120, "在铁匠铺永久强化攻击力",
                                             SpecialEffect::WeaponScroll, 5);
    }
    if (id == "铠甲强化卷") {
        return std::make_shared<SpecialItem>("铠甲强化卷", 110, "在铁匠铺永久强化防御力",
                                             SpecialEffect::ArmorScroll, 4);
    }
    if (id == "回血药") {
        return std::make_shared<SpecialItem>("回血药", 45, "按百分比恢复生命值",
                                             SpecialEffect::HpPotion, 40);
    }
    if (id == "回蓝药") {
        return std::make_shared<SpecialItem>("回蓝药", 35, "按百分比恢复蓝量",
                                             SpecialEffect::MpPotion, 50);
    }
    if (id == "狂暴药水") {
        return std::make_shared<SpecialItem>("狂暴药水", 80, "提升接下来3回合的攻击力",
                                             SpecialEffect::RagePotion, 8, 3);
    }
    if (id == "虚弱药水") {
        return std::make_shared<SpecialItem>("虚弱药水", 70, "降低当前敌人的攻击力",
                                             SpecialEffect::WeakPotion, 6, 3);
    }
    if (id == "疲惫药水") {
        return std::make_shared<SpecialItem>("疲惫药水", 75, "使敌人概率跳过回合",
                                             SpecialEffect::FatiguePotion, 0, 3);
    }
    if (id == "面包") return std::make_shared<Food>("面包", 15, "恢复少量HP", 25);
    if (id == "烤鸡") return std::make_shared<Food>("烤鸡", 35, "恢复较多HP", 60);
    if (id == "力量药水") return std::make_shared<Medicine>("力量药水", 50, "提升攻击力", "attack", 8, 3);
    if (id == "防御药水") return std::make_shared<Medicine>("防御药水", 45, "提升防御力", "defense", 6, 3);
    if (id == "急救包") return std::make_shared<Medicine>("急救包", 40, "立即恢复HP", "heal", 100, 1);
    if (id == "铁剑") return std::make_shared<Equipment>("铁剑", 80, "普通铁剑", 5, 0);
    if (id == "皮甲") return std::make_shared<Equipment>("皮甲", 70, "轻便皮甲", 0, 4);
    if (id == "勇者之剑") return std::make_shared<Equipment>("勇者之剑", 200, "传说中的宝剑", 15, 5);
    return nullptr;
}
