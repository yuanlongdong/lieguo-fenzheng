#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <chrono>

namespace lieguo {

// ============ 枚举 ============
enum class CityType { Capital, Fortress, Normal };
enum class GeneralQuality { Orange, Purple, Blue };
enum class GameState { Idle, Playing, Paused, Ended };
enum class GiftEffect { Join, Archer, Buff, General, Damage, Capture, Sabotage };
enum class BuffType { Attack, Defense, All };

// ============ Buff ============
struct Buff {
    BuffType type;
    double value;
    double duration;
    std::string source;
};

// ============ 武将 ============
struct General {
    std::string id;
    std::string name;
    GeneralQuality quality;
    std::string faction;
    std::string passive;
    std::string skill;
    std::string skillDesc;
    int skillDamage = 0;
    double skillBuff = 0;
    int skillDuration = 0;
    int skillSummon = 0;
    int skillHeal = 0;
    int skillCooldown = 60;
    int level = 1;
    double cooldown = 0;
};

// ============ 阵营 ============
struct Faction {
    std::string id;
    std::string name;
    std::string color;
    std::string trait;
    std::string traitDesc;
    std::string style;
    // 特性参数
    double attackBonusWhenStrong = 0;
    double durabilityBonus = 0;
    double freeOutputBonus = 0;
    double skillDamageBonus = 0;
    bool canRangedAttack = false;
    double defenseBonus = 0;
    double siegeDefenseBonus = 0;
    double archerBonus = 0;
    double bombDamageBonus = 0;
    // 动态状态
    int troops = 500;
    int archers = 0;
    int cavalry = 0;
    int playerCount = 0;
    std::vector<General> generals;
    std::vector<Buff> buffs;
    bool alive = true;
    int citiesOwned = 0;
    std::unordered_map<std::string, int> contribution;
    int totalContribution = 0;
    bool nextBombDouble = false;
};

// ============ 城池 ============
struct City {
    std::string id;
    std::string name;
    CityType type;
    std::string faction; // 空字符串表示中立
    int x = 0;
    int y = 0;
    std::vector<std::string> neighbors;
    int durability = 20000;
    int maxDurability = 20000;
    bool underAttack = false;
    std::string attacker;
};

// ============ 礼物 ============
struct Gift {
    std::string id;
    std::string name;
    int price;
    GiftEffect effect;
    int troops = 0;
    double buff = 0;
    int duration = 0;
    std::string quality; // "orange","purple","blue","random"
    int damage = 0;
    std::string desc;
};

// ============ 同盟 ============
struct Alliance {
    std::vector<std::string> members;
    double remaining = 600;
    double totalDuration = 600;
};

// ============ 战斗日志 ============
struct BattleLog {
    double time;
    std::string msg;
};

// ============ 游戏配置 ============
struct GameConfig {
    double gameSpeed = 1.0;
    int tickInterval = 1000; // ms
    int maxGenerals = 3;
    double allianceDuration = 600;
    double allianceUnlockTime = 1200;
    // 城池参数
    std::unordered_map<CityType, int> cityDurability = {
        {CityType::Capital, 50000},
        {CityType::Normal, 20000},
        {CityType::Fortress, 30000}
    };
    std::unordered_map<CityType, double> cityDefense = {
        {CityType::Capital, 1.5},
        {CityType::Normal, 1.2},
        {CityType::Fortress, 1.3}
    };
    std::unordered_map<CityType, int> cityProduction = {
        {CityType::Capital, 5},
        {CityType::Normal, 2},
        {CityType::Fortress, 4}
    };
    double durabilityDecayCoeff = 20;
    double siegeLossRate = 0.01;
    // 免费产出
    int freeLike = 1;
    int freeComment = 5;
    int freeJoin = 20;
    int freeShare = 100;
    // 人数平衡
    double overpopulatedRatio = 1.5;
    double underpopulatedRatio = 0.5;
    int overpopulatedBonus = 10;
    int underpopulatedBonus = 50;
};

// ============ 事件回调 ============
using EventCallback = std::function<void(const std::string&)>;

} // namespace lieguo
