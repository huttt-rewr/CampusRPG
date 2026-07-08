#ifndef ENEMY_H
#define ENEMY_H

#include <string>

class Character;

class Enemy {
protected:
    std::string name;
    int hp;
    int maxHp;
    int attack;
    int baseAttack;
    int defense;
    int expReward;
    int goldReward;

public:
    Enemy(const std::string& name, int hp, int atk, int def,
          int expR, int goldR);
    virtual ~Enemy() = default;

    virtual int attackPlayer() const;
    void takeDamage(int dmg);
    bool isAlive() const;
    int getExpReward() const;
    int getGoldReward() const;

    std::string getName() const;
    int getHp() const;
    int getMaxHp() const;
    int getAttack() const;
    int getDefense() const;
    void buffAttack(int amount);
    virtual void resetBattleState();
};

class NormalMonster : public Enemy {
    int fleeThreshold;
public:
    NormalMonster(const std::string& name, int hp, int atk, int def,
                  int expR, int goldR, int fleeHP = 20);
    int attackPlayer() const override;
    bool tryFlee() const;
};

class EliteMonster : public Enemy {
    double critRate;
    int armor;
public:
    EliteMonster(const std::string& name, int hp, int atk, int def,
                 int expR, int goldR, double crit = 0.25, int armor = 5);
    int attackPlayer() const override;
    bool isCrit() const;
};

class Boss : public Enemy {
    std::string skillName;
    int skillDamage;
    int phase;
public:
    Boss(const std::string& name, int hp, int atk, int def,
         int expR, int goldR, const std::string& skill, int skillDmg);
    int attackPlayer() const override;
    int useSkill() const;
    std::string getSkillName() const;
    void checkPhase();
    int getPhase() const;
    void resetBattleState() override;
};

#endif
