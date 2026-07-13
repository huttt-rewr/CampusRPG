// Profession.cpp
// 六个职业的基础属性和技能表实现，供 Qt 创建角色和战斗技能栏使用。
#include "Profession.h"

std::string StudentProfession::name() const { return "学生"; }
std::string StudentProfession::description() const { return "擅长学习不擅长作战，学习能力强。"; }
ProfessionStats StudentProfession::baseStats() const { return {120, 40, 100, 18, 8, 8, 6}; }
std::vector<SkillDef> StudentProfession::skills() const {
    return {
        {"含怒一击", 1, 0, 8, "敌方单体", "以125%物理攻击力攻击敌方一名指定角色，消耗一定hp。"},
        {"吃我一棒", 1, 0, 0, "敌方单体", "以100%物理攻击力攻击敌方一名指定角色。"},
        {"扫膛腿", 1, 0, 10, "敌方全体", "以50%物理攻击力攻击敌方全体在场人员，消耗一定hp。"}
    };
}
std::unique_ptr<Profession> StudentProfession::clone() const { return std::make_unique<StudentProfession>(*this); }

std::string IceMageProfession::name() const { return "冰法师"; }
std::string IceMageProfession::description() const { return "生命较低，伤害较高，蓝条充足。"; }
ProfessionStats IceMageProfession::baseStats() const { return {80, 150, 90, 8, 26, 4, 12}; }
std::vector<SkillDef> IceMageProfession::skills() const {
    return {
        {"冰锥术", 1, 0, 0, "敌方单体", "以100%法术攻击力召唤冰锥攻击敌方，并回复自身少量蓝量。"},
        {"冰冻术", 5, 18, 0, "敌方单体", "消耗少量蓝量，冻结一名敌人，并对其造成80%法术伤害。"},
        {"凛冬赐福", 10, 24, 0, "我方全体", "使我方单位接下来三个回合每一次攻击有50%概率使被攻击敌人被迟缓。"},
        {"凛冬领域", 15, 36, 0, "敌方全体", "对场上敌人造成80%法术伤害，未迟缓者附加迟缓，已有迟缓者直接冻结。"},
        {"凛冬降临", 20, 70, 50, "全场敌人", "以200%法术伤害攻击全场，禁止敌方行动一回合，自身减少一半血量并眩晕3回合。"}
    };
}
std::unique_ptr<Profession> IceMageProfession::clone() const { return std::make_unique<IceMageProfession>(*this); }

std::string PaladinProfession::name() const { return "圣骑士"; }
std::string PaladinProfession::description() const { return "生命较高，防御较高，伤害较低。"; }
ProfessionStats PaladinProfession::baseStats() const { return {160, 80, 95, 14, 10, 16, 12}; }
std::vector<SkillDef> PaladinProfession::skills() const {
    return {
        {"正义审判", 1, 0, 0, "敌方单体", "以100%物理攻击力攻击一名敌方。"},
        {"庇护坚壁", 5, 18, 0, "自身", "提升自己30%防御力，持续3回合。"},
        {"舍己", 10, 24, 0, "自身", "若圣骑士未退场，接下来三回合敌人只能攻击圣骑士。"},
        {"致盲", 15, 30, 0, "敌方全体", "使敌方下一回合无法攻击，并少量回复在场所有角色血量。"},
        {"圣光领域", 20, 55, 0, "敌方全体", "敌人受到80%圣骑士真实攻击力伤害，并使敌方该回合内防御力短暂清0。"}
    };
}
std::unique_ptr<Profession> PaladinProfession::clone() const { return std::make_unique<PaladinProfession>(*this); }

std::string BlesserProfession::name() const { return "祈福者"; }
std::string BlesserProfession::description() const { return "血量较低，奶妈定位。"; }
ProfessionStats BlesserProfession::baseStats() const { return {90, 140, 95, 7, 18, 6, 16}; }
std::vector<SkillDef> BlesserProfession::skills() const {
    return {
        {"祈福", 1, 16, 0, "队友单体", "回复一名队友生命值，不可回复自身。"},
        {"祈愿", 5, 14, 0, "队友单体", "回复一名队友蓝量，不可回复自身。"},
        {"祈甘霖", 10, 32, 30, "除自身外全体队友", "回复除自身外所有队友30%生命值。"},
        {"祈无忧", 15, 28, 0, "队友单体", "将队友身上所有debuff转移到自身身上。"},
        {"祈命", 20, 75, 100, "队友单体", "指定一名队友即将退场时满状态复活，并减少自身100%生命值。"}
    };
}
std::unique_ptr<Profession> BlesserProfession::clone() const { return std::make_unique<BlesserProfession>(*this); }

std::string BloodWarriorProfession::name() const { return "血战士"; }
std::string BloodWarriorProfession::description() const { return "攻击力高，防御力低，血量高。"; }
ProfessionStats BloodWarriorProfession::baseStats() const { return {180, 45, 100, 26, 5, 5, 5}; }
std::vector<SkillDef> BloodWarriorProfession::skills() const {
    return {
        {"燃血", 1, 0, 5, "敌方单体", "消耗自身5%血量，以120%物理攻击力攻击一名指定敌方。"},
        {"沸血", 5, 0, 10, "自身", "额外获得2次行动机会，这两次行动只能使用技能攻击。"},
        {"窃血", 10, 0, 0, "敌方单体", "以100%物理攻击力攻击一名指定敌方，并回复与造成伤害等值的血量。"},
        {"暴血", 15, 0, 30, "敌方全体", "对所有敌方造成300%物理攻击力伤害，并使自己眩晕一回合。"},
        {"无我", 20, 0, 100, "敌方全体", "自身血量不足50%时可开启，造成500%物理攻击力伤害，并永久降低该关卡敌方目前防御力的一半。"}
    };
}
std::unique_ptr<Profession> BloodWarriorProfession::clone() const { return std::make_unique<BloodWarriorProfession>(*this); }

std::string MagicianProfession::name() const { return "魔术师"; }
std::string MagicianProfession::description() const { return "攻击力高，防御力低，血量低。"; }
ProfessionStats MagicianProfession::baseStats() const { return {85, 120, 90, 10, 28, 4, 10}; }
std::vector<SkillDef> MagicianProfession::skills() const {
    return {
        {"火焰魔术", 1, 12, 0, "随机敌人", "无视嘲讽，对随机一名敌人造成80%法术攻击力伤害，并附加3回合灼烧。"},
        {"镜面魔术", 5, 24, 0, "队友单体", "为一名队友附加镜子，承担伤害并按规则反弹。"},
        {"箱中魔术", 10, 35, 0, "敌方单体", "80%概率困住普通敌人3回合，精英50%，首领30%，失败返还部分蓝量。"},
        {"障眼魔术", 15, 38, 0, "自身和敌方单体", "双方陷入光学迷宫3回合，无法行动且不受场上效果，首领50%概率成功。"},
        {"同命巫术", 20, 45, 0, "自身和敌方单体", "获得嘲讽并指定敌人，2回合内双方无法死亡，结算累计伤害。"}
    };
}
std::unique_ptr<Profession> MagicianProfession::clone() const { return std::make_unique<MagicianProfession>(*this); }
