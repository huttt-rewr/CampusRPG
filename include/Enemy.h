// Enemy.h
// 敌人继承体系：敌人基类定义纯虚攻击逻辑，普通、精英、BOSS 敌人分别重写。
#ifndef ENEMY_H
#define ENEMY_H

#include <memory>
#include <string>

class Character;

class Enemy {
protected:
    std::string name_;
    int maxHp_;
    int currentHp_;
    int attack_;
    int defense_;
    int rewardExp_;
    int rewardGold_;
    std::string dropItemName_;
    int dropRate_;

public:
    Enemy(std::string name, int hp, int attack, int defense,
          int rewardExp, int rewardGold, std::string dropItemName, int dropRate);
    virtual ~Enemy() = default;

    const std::string& getName() const;
    int getCurrentHp() const;
    int getMaxHp() const;
    int getAttack() const;
    int getDefense() const;
    int getRewardExp() const;
    int getRewardGold() const;
    const std::string& getDropItemName() const;
    int getDropRate() const;
    bool isAlive() const;
    void takeDamage(int amount);
    void reset();
    virtual int attackPlayer(Character& player, int round) = 0;
    virtual std::string kind() const = 0;
    virtual std::unique_ptr<Enemy> clone() const = 0;
    std::string info() const;
};

class NormalEnemy : public Enemy {
public:
    NormalEnemy();
    int attackPlayer(Character& player, int round) override;
    std::string kind() const override;
    std::unique_ptr<Enemy> clone() const override;
};

class EliteEnemy : public Enemy {
public:
    EliteEnemy();
    int attackPlayer(Character& player, int round) override;
    std::string kind() const override;
    std::unique_ptr<Enemy> clone() const override;
};

class BossEnemy : public Enemy {
public:
    BossEnemy();
    int attackPlayer(Character& player, int round) override;
    std::string kind() const override;
    std::unique_ptr<Enemy> clone() const override;
};

#endif
