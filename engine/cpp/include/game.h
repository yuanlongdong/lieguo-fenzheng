#pragma once
#include "types.h"
#include "config.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>

namespace lieguo {

class Game {
public:
    Game();
    ~Game();

    // ===== 生命周期 =====
    void init();
    void start();
    void pause();
    void resume();
    void stop();

    // ===== 主循环（单步推进，由外部调用或内部线程驱动）=====
    void tick(); // 推进一个游戏帧

    // ===== 玩家操作 =====
    bool joinFaction(const std::string& factionId);
    bool like();
    bool comment(const std::string& text);
    struct GiftResult { bool success; std::string msg; };
    GiftResult sendGift(const std::string& giftId, const std::string& targetCityId = "");

    // ===== 外交 =====
    bool createAlliance(const std::vector<std::string>& members);
    bool breakAlliance(const std::string& factionId);
    bool isAllied(const std::string& f1, const std::string& f2) const;

    // ===== 事件 =====
    std::string triggerRandomEvent();

    // ===== 查询 =====
    GameState getState() const { return state_; }
    double getElapsedTime() const { return elapsedTime_; }
    const std::unordered_map<std::string, Faction>& getFactions() const { return factions_; }
    const std::vector<City>& getCities() const { return cities_; }
    const std::vector<Alliance>& getAlliances() const { return alliances_; }
    const std::vector<BattleLog>& getBattleLogs() const { return battleLogs_; }
    const std::string& getPlayerFaction() const { return playerFaction_; }
    int getHuFu() const { return huFu_; }
    const std::string& getWinner() const { return winner_; }
    City* findCity(const std::string& id);
    Faction* findFaction(const std::string& id);

    // ===== 事件订阅 =====
    using LogCallback = std::function<void(const std::string&)>;
    void onLog(LogCallback cb) { logCallback_ = cb; }
    using UpdateCallback = std::function<void()>;
    void onUpdate(UpdateCallback cb) { updateCallback_ = cb; }

    // ===== 工具 =====
    std::string formatTime() const;

private:
    // ===== 内部更新 =====
    void produceTroops();
    void processSieges();
    void attackCity(const std::string& factionId, City& city);
    double calcAttackPower(const std::string& factionId) const;
    double calcDefensePower(const City& city) const;
    void captureCity(const std::string& factionId, City& city);
    void eliminateFaction(const std::string& factionId);
    void updateGenerals();
    void castGeneralSkill(const std::string& factionId, General& general);
    City* findNearestEnemyCity(const std::string& factionId);
    void updateBuffs();
    void updateAlliances();
    void regenerateDurability();
    void updateCityCounts();
    void checkVictory();
    void summonGeneral(const std::string& factionId, const std::string& quality);
    void addContribution(const std::string& factionId, const std::string& playerId, int amount);
    void addLog(const std::string& msg);

    // ===== 数据 =====
    GameState state_ = GameState::Idle;
    int tickCount_ = 0;
    double elapsedTime_ = 0;
    std::unordered_map<std::string, Faction> factions_;
    std::vector<City> cities_;
    std::vector<Alliance> alliances_;
    std::vector<BattleLog> battleLogs_;
    std::string playerFaction_;
    int huFu_ = 0;
    std::string winner_;
    std::vector<General> generalPool_;
    std::vector<Gift> giftList_;

    // ===== 回调 =====
    LogCallback logCallback_;
    UpdateCallback updateCallback_;

    // ===== 线程安全 =====
    mutable std::mutex mutex_;
};

} // namespace lieguo
