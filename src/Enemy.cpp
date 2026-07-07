#include "Enemy.h"
#include <cstdlib>

Enemy::Enemy(const std::string& n, int h, int a, int d, int er, int gr)
    : name(n), hp(h), maxHp(h), attack(a), defense(d),
      expReward(er), goldReward(gr) {}

int Enemy::attackPlayer() const { return attack; }
void Enemy::takeDamage(int dmg) {
    hp = std::max(0, hp - std::max(1, dmg - defense / 3));
}
bool Enemy::isAlive() const { return hp > 0; }
int Enemy::getExpReward() const { return expReward; }
int Enemy::getGoldReward() const { return goldReward; }
std::string Enemy::getName() const { return name; }
int Enemy::getHp() const { return hp; }
int Enemy::getMaxHp() const { return maxHp; }
int Enemy::getAttack() const { return attack; }
int Enemy::getDefense() const { return defense; }

// NormalMonster
NormalMonster::NormalMonster(const std::string& n, int h, int a, int d,
                             int er, int gr, int ft)
    : Enemy(n, h, a, d, er, gr), fleeThreshold(ft) {}

int NormalMonster::attackPlayer() const {
    if (tryFlee()) return -(attack);  // negative = fleeing
    return attack;
}
bool NormalMonster::tryFlee() const {
    return (hp * 100 / maxHp) < fleeThreshold;
}

// EliteMonster
EliteMonster::EliteMonster(const std::string& n, int h, int a, int d,
                           int er, int gr, double cr, int ar)
    : Enemy(n, h, a, d, er, gr), critRate(cr), armor(ar) {}

int EliteMonster::attackPlayer() const {
    if (isCrit()) return attack * 2;
    return attack + armor / 2;
}
bool EliteMonster::isCrit() const {
    return (rand() % 100) < (int)(critRate * 100);
}

// Boss
Boss::Boss(const std::string& n, int h, int a, int d,
           int er, int gr, const std::string& sn, int sd)
    : Enemy(n, h, a, d, er, gr), skillName(sn), skillDamage(sd), phase(1) {}

int Boss::attackPlayer() const {
    if (rand() % 100 < 30 + phase * 10) return useSkill();
    return attack + phase * 5;
}
int Boss::useSkill() const { return skillDamage + phase * 10; }
std::string Boss::getSkillName() const { return skillName; }
void Boss::checkPhase() {
    if (hp * 100 / maxHp < 50 && phase == 1) {
        const_cast<Boss*>(this)->phase = 2;
        const_cast<Boss*>(this)->attack += 10;
    }
}
int Boss::getPhase() const { return phase; }
