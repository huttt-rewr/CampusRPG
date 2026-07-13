// Profession.h
// 职业抽象基类与技能数据定义：六个校园 RPG 职业通过继承和多态提供不同属性、技能。
#ifndef PROFESSION_H
#define PROFESSION_H

#include <memory>
#include <string>
#include <vector>

struct SkillDef {
    std::string name;
    int unlockLevel;
    int mpCost;
    int hpCostPercent;
    std::string targetType;
    std::string description;
};

struct ProfessionStats {
    int hp;
    int mp;
    int vigor;
    int physicalAttack;
    int magicAttack;
    int physicalDefense;
    int magicResistance;
};

class Profession {
public:
    virtual ~Profession() = default;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual ProfessionStats baseStats() const = 0;
    virtual std::vector<SkillDef> skills() const = 0;
    virtual std::unique_ptr<Profession> clone() const = 0;
};

class StudentProfession : public Profession {
public:
    std::string name() const override;
    std::string description() const override;
    ProfessionStats baseStats() const override;
    std::vector<SkillDef> skills() const override;
    std::unique_ptr<Profession> clone() const override;
};

class IceMageProfession : public Profession {
public:
    std::string name() const override;
    std::string description() const override;
    ProfessionStats baseStats() const override;
    std::vector<SkillDef> skills() const override;
    std::unique_ptr<Profession> clone() const override;
};

class PaladinProfession : public Profession {
public:
    std::string name() const override;
    std::string description() const override;
    ProfessionStats baseStats() const override;
    std::vector<SkillDef> skills() const override;
    std::unique_ptr<Profession> clone() const override;
};

class BlesserProfession : public Profession {
public:
    std::string name() const override;
    std::string description() const override;
    ProfessionStats baseStats() const override;
    std::vector<SkillDef> skills() const override;
    std::unique_ptr<Profession> clone() const override;
};

class BloodWarriorProfession : public Profession {
public:
    std::string name() const override;
    std::string description() const override;
    ProfessionStats baseStats() const override;
    std::vector<SkillDef> skills() const override;
    std::unique_ptr<Profession> clone() const override;
};

class MagicianProfession : public Profession {
public:
    std::string name() const override;
    std::string description() const override;
    ProfessionStats baseStats() const override;
    std::vector<SkillDef> skills() const override;
    std::unique_ptr<Profession> clone() const override;
};

#endif
