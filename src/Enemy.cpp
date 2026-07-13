// Enemy.cpp
// 敌人类实现：通过基类指针调用不同敌人的攻击逻辑，体现运行时多态。
#include "Enemy.h"
#include "Character.h"
#include <algorithm>
#include <sstream>

Enemy::Enemy(std::string name, int hp, int attack, int defense,
             int rewardExp, int rewardGold, std::string dropItemName, int dropRate)
    : name_(std::move(name)), maxHp_(hp), currentHp_(hp), attack_(attack), defense_(defense),
      rewardExp_(rewardExp), rewardGold_(rewardGold), dropItemName_(std::move(dropItemName)), dropRate_(dropRate) {}

const std::string& Enemy::getName() const { return name_; }
int Enemy::getCurrentHp() const { return currentHp_; }
int Enemy::getMaxHp() const { return maxHp_; }
int Enemy::getAttack() const { return attack_; }
int Enemy::getDefense() const { return defense_; }
int Enemy::getRewardExp() const { return rewardExp_; }
int Enemy::getRewardGold() const { return rewardGold_; }
const std::string& Enemy::getDropItemName() const { return dropItemName_; }
int Enemy::getDropRate() const { return dropRate_; }
bool Enemy::isAlive() const { return currentHp_ > 0; }
void Enemy::takeDamage(int amount) { currentHp_ = std::max(0, currentHp_ - std::max(1, amount)); }
void Enemy::reset() { currentHp_ = maxHp_; }

std::string Enemy::info() const {
    std::ostringstream out;
    out << kind() << "：" << name_ << "  生命 " << maxHp_
        << "  攻击 " << attack_ << "  防御 " << defense_
        << "  奖励 " << rewardExp_ << "经验/" << rewardGold_ << "金币";
    return out.str();
}

NormalEnemy::NormalEnemy()
    : Enemy("校园占座狗", 80, 16, 4, 40, 30, "校园食堂盒饭", 45) {}

int NormalEnemy::attackPlayer(Character& player, int) {
    int damage = std::max(1, attack_ - player.getDefense());
    player.takeDamage(damage);
    return damage;
}

std::string NormalEnemy::kind() const { return "普通敌人"; }
std::unique_ptr<Enemy> NormalEnemy::clone() const { return std::make_unique<NormalEnemy>(*this); }

EliteEnemy::EliteEnemy()
    : Enemy("教务处查课老师", 130, 24, 8, 90, 70, "校医院退烧药", 55) {}

int EliteEnemy::attackPlayer(Character& player, int round) {
    int rawAttack = attack_ + (round % 3 == 0 ? 10 : 0);
    int damage = std::max(1, rawAttack - player.getDefense());
    player.takeDamage(damage);
    return damage;
}

std::string EliteEnemy::kind() const { return "精英敌人"; }
std::unique_ptr<Enemy> EliteEnemy::clone() const { return std::make_unique<EliteEnemy>(*this); }

BossEnemy::BossEnemy()
    : Enemy("期末挂科神龙", 220, 34, 12, 180, 160, "图书馆占座笔", 80) {}

int BossEnemy::attackPlayer(Character& player, int round) {
    int rawAttack = attack_ + (round % 4 == 0 ? 20 : 0);
    int damage = std::max(1, rawAttack - player.getDefense());
    player.takeDamage(damage);
    return damage;
}

std::string BossEnemy::kind() const { return "BOSS敌人"; }
std::unique_ptr<Enemy> BossEnemy::clone() const { return std::make_unique<BossEnemy>(*this); }
