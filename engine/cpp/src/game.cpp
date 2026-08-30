#include "game.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>
#include <iomanip>

namespace lieguo {

static std::mt19937 rng(std::random_device{}());

Game::Game() { init(); }
Game::~Game() { stop(); }

void Game::init() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = GameState::Idle;
    tickCount_ = 0;
    elapsedTime_ = 0;
    playerFaction_.clear();
    huFu_ = 0;
    winner_.clear();
    alliances_.clear();
    battleLogs_.clear();

    auto factionTemplates = createFactions();
    factions_.clear();
    for (auto& f : factionTemplates) {
        f.troops = 500;
        f.archers = 0;
        f.cavalry = 0;
        f.playerCount = 0;
        f.generals.clear();
        f.buffs.clear();
        f.alive = true;
        f.citiesOwned = 0;
        f.contribution.clear();
        f.totalContribution = 0;
        factions_[f.id] = f;
    }

    cities_ = createCities();
    generalPool_ = getGeneralPool();
    giftList_ = getGifts();
    updateCityCounts();
}

void Game::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != GameState::Idle && state_ != GameState::Ended) return;
    state_ = GameState::Playing;
    tickCount_ = 0;
    elapsedTime_ = 0;
    addLog("列国纷争开始！七雄并立，逐鹿中原！");
}

void Game::pause() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ == GameState::Playing) state_ = GameState::Paused;
}

void Game::resume() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ == GameState::Paused) state_ = GameState::Playing;
}

void Game::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = GameState::Idle;
}

void Game::tick() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != GameState::Playing) return;

    produceTroops();
    processSieges();
    updateGenerals();
    updateBuffs();
    updateAlliances();
    regenerateDurability();

    tickCount_++;
    elapsedTime_ += CONFIG.tickInterval / 1000.0 * CONFIG.gameSpeed;
    checkVictory();

    if (updateCallback_) updateCallback_();
}

void Game::produceTroops() {
    for (auto& city : cities_) {
        if (!city.faction.empty()) {
            auto it = factions_.find(city.faction);
            if (it != factions_.end() && it->second.alive) {
                it->second.troops += CONFIG.cityProduction[city.type];
            }
        }
    }
}

void Game::processSieges() {
    for (auto& [factionId, faction] : factions_) {
        if (!faction.alive || faction.troops < 100) continue;
        for (auto& ownCity : cities_) {
            if (ownCity.faction != factionId) continue;
            for (const auto& neighborId : ownCity.neighbors) {
                auto target = findCity(neighborId);
                if (!target || target->faction == factionId) continue;
                if (!target->faction.empty() && isAllied(factionId, target->faction)) continue;
                attackCity(factionId, *target);
            }
        }
    }
}

void Game::attackCity(const std::string& factionId, City& city) {
    auto faction = findFaction(factionId);
    if (!faction || faction->troops <= 0) return;

    double attackPower = calcAttackPower(factionId);
    double defensePower = calcDefensePower(city);

    if (attackPower > defensePower) {
        double decay = (attackPower - defensePower) / CONFIG.durabilityDecayCoeff;
        if (!city.faction.empty()) {
            auto defender = findFaction(city.faction);
            if (defender && defender->siegeDefenseBonus > 0) {
                decay *= (1 - defender->siegeDefenseBonus);
            }
        }
        city.durability = std::max(0, static_cast<int>(city.durability - decay));
        city.underAttack = true;
        city.attacker = factionId;

        int loss = static_cast<int>(faction->troops * CONFIG.siegeLossRate);
        faction->troops = std::max(0, faction->troops - loss);

        auto baiqi = std::find_if(faction->generals.begin(), faction->generals.end(),
            [](const General& g){ return g.id == "baiqi"; });
        if (baiqi != faction->generals.end()) {
            faction->troops += static_cast<int>(loss * 0.2);
        }

        if (city.durability <= 0) {
            captureCity(factionId, city);
        }
    } else {
        city.underAttack = false;
        city.attacker.clear();
    }
}

double Game::calcAttackPower(const std::string& factionId) const {
    auto it = factions_.find(factionId);
    if (it == factions_.end()) return 0;
    const auto& faction = it->second;
    double power = faction.troops + faction.archers * 1.25 + faction.cavalry * 1.1;

    if (faction.attackBonusWhenStrong > 0 && faction.troops > 5000) {
        power *= (1 + faction.attackBonusWhenStrong);
    }
    if (faction.archerBonus > 0) {
        power += faction.archers * faction.archerBonus * 0.25;
    }
    for (const auto& g : faction.generals) {
        if (g.id == "wuqi") power *= 1.30;
        if (g.id == "wangjian") power *= 1.30;
        if (g.id == "leyi") {
            double bonus = std::min(0.50, faction.citiesOwned * 0.10);
            power *= (1 + bonus);
        }
    }
    for (const auto& buff : faction.buffs) {
        if (buff.type == BuffType::Attack || buff.type == BuffType::All) {
            power *= (1 + buff.value);
        }
    }
    return power;
}

double Game::calcDefensePower(const City& city) const {
    if (city.faction.empty()) return 100;
    auto it = factions_.find(city.faction);
    if (it == factions_.end()) return 100;
    const auto& faction = it->second;
    double power = (faction.troops / std::max(1, faction.citiesOwned)) * CONFIG.cityDefense[city.type];

    if (faction.defenseBonus > 0) power *= (1 + faction.defenseBonus);
    for (const auto& g : faction.generals) {
        if (g.id == "lianpo") power *= 1.30;
        if (g.id == "limu") power *= 1.50;
    }
    for (const auto& buff : faction.buffs) {
        if (buff.type == BuffType::Defense || buff.type == BuffType::All) {
            power *= (1 + buff.value);
        }
    }
    return power;
}

void Game::captureCity(const std::string& factionId, City& city) {
    std::string oldFaction = city.faction;
    city.faction = factionId;
    city.durability = static_cast<int>(city.maxDurability * 0.5);
    city.underAttack = false;
    city.attacker.clear();
    updateCityCounts();

    auto f = findFaction(factionId);
    addLog((f ? f->name : factionId) + "国攻占了" + city.name + "！");

    if (!oldFaction.empty()) {
        int oldCount = 0;
        for (const auto& c : cities_) {
            if (c.faction == oldFaction) oldCount++;
        }
        if (oldCount == 0) eliminateFaction(oldFaction);
    }
}

void Game::eliminateFaction(const std::string& factionId) {
    auto faction = findFaction(factionId);
    if (!faction) return;
    faction->alive = false;
    faction->troops = 0;
    faction->generals.clear();
    for (auto& city : cities_) {
        if (city.faction == factionId) {
            city.faction.clear();
            city.durability = static_cast<int>(city.maxDurability * 0.3);
        }
    }
    addLog(faction->name + "国灭亡！遗民可加入其他阵营。");
}

void Game::updateGenerals() {
    for (auto& [factionId, faction] : factions_) {
        for (auto& general : faction.generals) {
            if (general.cooldown > 0) {
                general.cooldown -= CONFIG.tickInterval / 1000.0 * CONFIG.gameSpeed;
                if (general.cooldown <= 0) {
                    general.cooldown = 0;
                    castGeneralSkill(factionId, general);
                }
            }
        }
    }
}

void Game::castGeneralSkill(const std::string& factionId, General& general) {
    auto faction = findFaction(factionId);
    if (!faction) return;

    double damageMultiplier = 1.0;
    if (faction->skillDamageBonus > 0) damageMultiplier = 1 + faction->skillDamageBonus;

    auto jingke = std::find_if(faction->generals.begin(), faction->generals.end(),
        [](const General& g){ return g.id == "jingke"; });
    if (jingke != faction->generals.end() && general.id != "jingke") {
        damageMultiplier *= 1.2;
    }

    if (general.skillDamage > 0) {
        City* target = findNearestEnemyCity(factionId);
        if (target) {
            int dmg = static_cast<int>(general.skillDamage * damageMultiplier);
            target->durability = std::max(0, target->durability - dmg);
            addLog(general.name + "释放【" + general.skill + "】，对" + target->name + "造成" + std::to_string(dmg) + "伤害！");
            if (target->durability <= 0) captureCity(factionId, *target);
        }
    }
    if (general.skillSummon > 0) {
        if (general.id == "limu") faction->cavalry += general.skillSummon;
        else faction->troops += general.skillSummon;
        addLog(general.name + "释放【" + general.skill + "】，获得" + std::to_string(general.skillSummon) + "兵力！");
    }
    if (general.skillHeal > 0) {
        std::vector<City*> ownCities;
        for (auto& c : cities_) {
            if (c.faction == factionId) ownCities.push_back(&c);
        }
        if (!ownCities.empty()) {
            int healPerCity = general.skillHeal / static_cast<int>(ownCities.size());
            for (auto* c : ownCities) {
                c->durability = std::min(c->maxDurability, c->durability + healPerCity);
            }
        }
        addLog(general.name + "释放【" + general.skill + "】，城池耐久回复！");
    }
    if (general.skillBuff > 0) {
        faction->buffs.push_back({
            BuffType::All, general.skillBuff,
            static_cast<double>(general.skillDuration > 0 ? general.skillDuration : 20),
            general.name
        });
        addLog(general.name + "释放【" + general.skill + "】，全属性提升！");
    }

    double cooldown = general.skillCooldown > 0 ? general.skillCooldown : 60;
    auto sunbin = std::find_if(faction->generals.begin(), faction->generals.end(),
        [](const General& g){ return g.id == "sunbin"; });
    if (sunbin != faction->generals.end() && general.id != "sunbin") cooldown *= 0.7;
    auto shenbuhai = std::find_if(faction->generals.begin(), faction->generals.end(),
        [](const General& g){ return g.id == "shenbuhai"; });
    if (shenbuhai != faction->generals.end()) cooldown *= 0.8;
    general.cooldown = cooldown;
}

City* Game::findNearestEnemyCity(const std::string& factionId) {
    for (auto& ownCity : cities_) {
        if (ownCity.faction != factionId) continue;
        for (const auto& nid : ownCity.neighbors) {
            auto target = findCity(nid);
            if (target && !target->faction.empty() && target->faction != factionId) {
                return target;
            }
        }
    }
    for (auto& ownCity : cities_) {
        if (ownCity.faction != factionId) continue;
        for (const auto& nid : ownCity.neighbors) {
            auto target = findCity(nid);
            if (target && target->faction != factionId) return target;
        }
    }
    return nullptr;
}

void Game::updateBuffs() {
    for (auto& [fid, faction] : factions_) {
        faction.buffs.erase(std::remove_if(faction.buffs.begin(), faction.buffs.end(),
            [this](Buff& buff) {
                buff.duration -= CONFIG.tickInterval / 1000.0 * CONFIG.gameSpeed;
                return buff.duration <= 0;
            }), faction.buffs.end());
    }
}

void Game::updateAlliances() {
    alliances_.erase(std::remove_if(alliances_.begin(), alliances_.end(),
        [this](Alliance& ally) {
            ally.remaining -= CONFIG.tickInterval / 1000.0 * CONFIG.gameSpeed;
            if (ally.remaining <= 0) {
                std::string names;
                for (const auto& m : ally.members) {
                    auto f = findFaction(m);
                    if (!names.empty()) names += "、";
                    names += f ? f->name : m;
                }
                addLog(names + "的同盟到期解散！");
                return true;
            }
            return false;
        }), alliances_.end());
}

void Game::regenerateDurability() {
    for (auto& city : cities_) {
        if (!city.faction.empty() && !city.underAttack) {
            int regen = 10;
            auto faction = findFaction(city.faction);
            if (faction) {
                auto quyuan = std::find_if(faction->generals.begin(), faction->generals.end(),
                    [](const General& g){ return g.id == "quyuan"; });
                if (quyuan != faction->generals.end()) regen *= 2;
            }
            city.durability = std::min(city.maxDurability, city.durability + regen);
        }
    }
}

void Game::updateCityCounts() {
    for (auto& [fid, faction] : factions_) {
        int count = 0;
        for (const auto& c : cities_) {
            if (c.faction == fid) count++;
        }
        faction.citiesOwned = count;
    }
}

void Game::checkVictory() {
    int aliveCount = 0;
    std::string lastAlive;
    for (const auto& [fid, faction] : factions_) {
        if (faction.alive) { aliveCount++; lastAlive = fid; }
    }
    if (aliveCount == 1) {
        state_ = GameState::Ended;
        winner_ = lastAlive;
        auto f = findFaction(lastAlive);
        addLog((f ? f->name : lastAlive) + "国一统天下！游戏结束！");
    }
}

bool Game::isAllied(const std::string& f1, const std::string& f2) const {
    for (const auto& a : alliances_) {
        bool hasF1 = std::find(a.members.begin(), a.members.end(), f1) != a.members.end();
        bool hasF2 = std::find(a.members.begin(), a.members.end(), f2) != a.members.end();
        if (hasF1 && hasF2) return true;
    }
    return false;
}

bool Game::createAlliance(const std::vector<std::string>& members) {
    if (members.size() < 2 || members.size() > 3) return false;
    for (const auto& m : members) {
        for (const auto& a : alliances_) {
            if (std::find(a.members.begin(), a.members.end(), m) != a.members.end()) {
                return false;
            }
        }
    }
    Alliance ally;
    ally.members = members;
    ally.remaining = CONFIG.allianceDuration;
    ally.totalDuration = CONFIG.allianceDuration;
    alliances_.push_back(ally);

    std::string names;
    for (const auto& m : members) {
        auto f = findFaction(m);
        if (!names.empty()) names += "、";
        names += f ? f->name : m;
    }
    addLog(names + "结成合纵同盟！共同抗敌！");
    return true;
}

bool Game::breakAlliance(const std::string& factionId) {
    for (auto it = alliances_.begin(); it != alliances_.end(); ++it) {
        auto mit = std::find(it->members.begin(), it->members.end(), factionId);
        if (mit != it->members.end()) {
            it->members.erase(mit);
            if (it->members.size() < 2) {
                alliances_.erase(it);
            }
            auto faction = findFaction(factionId);
            if (faction) {
                faction->buffs.push_back({BuffType::All, -0.20, 300.0, "背盟惩罚"});
                addLog(faction->name + "国背盟！全属性-20%持续5分钟。");
            }
            return true;
        }
    }
    return false;
}

bool Game::joinFaction(const std::string& factionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = factions_.find(factionId);
    if (it == factions_.end() || !it->second.alive) return false;

    playerFaction_ = factionId;
    it->second.playerCount++;

    double avgPlayers = 0;
    for (const auto& [fid, f] : factions_) avgPlayers += f.playerCount;
    avgPlayers /= 7.0;

    int bonus = CONFIG.freeJoin;
    if (it->second.playerCount > avgPlayers * CONFIG.overpopulatedRatio) {
        bonus = CONFIG.overpopulatedBonus;
    } else if (it->second.playerCount < avgPlayers * CONFIG.underpopulatedRatio) {
        bonus = CONFIG.underpopulatedBonus;
    }
    it->second.troops += bonus;
    addLog("你加入了" + it->second.name + "国，获得" + std::to_string(bonus) + "兵力！");
    return true;
}

bool Game::like() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (playerFaction_.empty()) return false;
    auto faction = findFaction(playerFaction_);
    if (!faction) return false;

    int amount = CONFIG.freeLike;
    if (faction->freeOutputBonus > 0) {
        amount = static_cast<int>(amount * (1 + faction->freeOutputBonus));
    }
    faction->troops += amount;
    addContribution(playerFaction_, "player", amount);
    return true;
}

bool Game::comment(const std::string& text) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (playerFaction_.empty()) return false;
    auto faction = findFaction(playerFaction_);
    if (!faction) return false;
    faction->troops += CONFIG.freeComment;
    addContribution(playerFaction_, "player", CONFIG.freeComment);
    addLog("弹幕: " + text);
    return true;
}

Game::GiftResult Game::sendGift(const std::string& giftId, const std::string& targetCityId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (playerFaction_.empty()) return {false, "请先加入阵营"};

    auto giftIt = std::find_if(giftList_.begin(), giftList_.end(),
        [&](const Gift& g){ return g.id == giftId; });
    if (giftIt == giftList_.end()) return {false, "礼物不存在"};

    const Gift& gift = *giftIt;
    auto faction = findFaction(playerFaction_);
    if (!faction) return {false, "阵营不存在"};

    switch (gift.effect) {
        case GiftEffect::Join:
            faction->troops += gift.troops;
            addLog("送出" + gift.name + "，+" + std::to_string(gift.troops) + "兵力");
            break;
        case GiftEffect::Archer:
            faction->archers += gift.troops;
            addLog("送出" + gift.name + "，+" + std::to_string(gift.troops) + "弓兵");
            break;
        case GiftEffect::Buff:
            faction->buffs.push_back({
                gift.id == "mirror" ? BuffType::All : BuffType::Attack,
                gift.buff, static_cast<double>(gift.duration), gift.name
            });
            addLog("送出" + gift.name + "，阵营获得增益！");
            break;
        case GiftEffect::General:
            summonGeneral(playerFaction_, gift.quality);
            if (gift.troops > 0) faction->troops += gift.troops;
            if (gift.buff > 0) {
                faction->buffs.push_back({BuffType::All, gift.buff,
                    static_cast<double>(gift.duration), gift.name});
            }
            break;
        case GiftEffect::Damage: {
            std::string targetId = targetCityId;
            if (targetId.empty()) {
                City* target = findNearestEnemyCity(playerFaction_);
                if (target) targetId = target->id;
            }
            if (!targetId.empty()) {
                City* city = findCity(targetId);
                if (city) {
                    int dmg = gift.damage;
                    if (faction->bombDamageBonus > 0) {
                        dmg = static_cast<int>(dmg * (1 + faction->bombDamageBonus));
                    }
                    if (faction->nextBombDouble) {
                        dmg *= 2;
                        faction->nextBombDouble = false;
                    }
                    city->durability = std::max(0, city->durability - dmg);
                    addLog("送出" + gift.name + "，对" + city->name + "造成" + std::to_string(dmg) + "伤害！");
                    if (city->durability <= 0) captureCity(playerFaction_, *city);
                }
            }
            break;
        }
        case GiftEffect::Capture: {
            if (!targetCityId.empty()) {
                City* city = findCity(targetCityId);
                if (city && city->type != CityType::Capital &&
                    city->durability < city->maxDurability * 0.5) {
                    captureCity(playerFaction_, *city);
                    addLog("嘉年华！直接夺取" + city->name + "！");
                } else {
                    return {false, "只能夺取耐久低于50%的非首都城池"};
                }
            }
            break;
        }
        case GiftEffect::Sabotage: {
            std::vector<Faction*> alliedEnemies;
            for (auto& [fid, f] : factions_) {
                if (f.alive && fid != playerFaction_) {
                    for (const auto& a : alliances_) {
                        if (std::find(a.members.begin(), a.members.end(), fid) != a.members.end()) {
                            alliedEnemies.push_back(&f);
                            break;
                        }
                    }
                }
            }
            if (!alliedEnemies.empty() && rng() % 2 == 0) {
                Faction* target = alliedEnemies[rng() % alliedEnemies.size()];
                breakAlliance(target->id);
                faction->troops += static_cast<int>(target->troops * 0.10);
                addLog("离间计成功！" + target->name + "国退出同盟，获得其10%兵力！");
            } else {
                addLog("离间计失败...");
            }
            break;
        }
    }

    addContribution(playerFaction_, "player", gift.price * 10);
    huFu_ += gift.price / 100;
    return {true, ""};
}

void Game::summonGeneral(const std::string& factionId, const std::string& quality) {
    auto faction = findFaction(factionId);
    if (!faction) return;

    std::vector<General> pool;
    if (quality == "orange") {
        for (const auto& g : generalPool_) if (g.quality == GeneralQuality::Orange) pool.push_back(g);
    } else if (quality == "purple") {
        for (const auto& g : generalPool_)
            if (g.quality == GeneralQuality::Purple || g.quality == GeneralQuality::Orange) pool.push_back(g);
    } else {
        if (rng() % 10 < 7) {
            for (const auto& g : generalPool_) if (g.quality == GeneralQuality::Blue) pool.push_back(g);
        } else {
            for (const auto& g : generalPool_) if (g.quality == GeneralQuality::Purple) pool.push_back(g);
        }
    }
    if (pool.empty()) return;

    General general = pool[rng() % pool.size()];
    general.level = 1;
    general.cooldown = 30;

    auto existing = std::find_if(faction->generals.begin(), faction->generals.end(),
        [&](const General& g){ return g.id == general.id; });
    if (existing != faction->generals.end()) {
        existing->level = std::min(3, existing->level + 1);
        addLog(general.name + "升级到" + std::to_string(existing->level) + "级！");
    } else {
        if (static_cast<int>(faction->generals.size()) >= CONFIG.maxGenerals) {
            auto qualityOrder = [](GeneralQuality q) {
                return q == GeneralQuality::Blue ? 1 : q == GeneralQuality::Purple ? 2 : 3;
            };
            std::sort(faction->generals.begin(), faction->generals.end(),
                [&](const General& a, const General& b) {
                    return qualityOrder(a.quality) < qualityOrder(b.quality);
                });
            faction->generals.erase(faction->generals.begin());
        }
        faction->generals.push_back(general);
        addLog(faction->name + "国召唤出" + general.name + "！");
    }
}

void Game::addContribution(const std::string& factionId, const std::string& playerId, int amount) {
    auto faction = findFaction(factionId);
    if (!faction) return;
    faction->contribution[playerId] += amount;
    faction->totalContribution += amount;
}

void Game::addLog(const std::string& msg) {
    BattleLog log;
    log.time = elapsedTime_;
    log.msg = msg;
    battleLogs_.insert(battleLogs_.begin(), log);
    if (battleLogs_.size() > 50) battleLogs_.pop_back();
    if (logCallback_) logCallback_(msg);
}

std::string Game::triggerRandomEvent() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> eventIds = {"disaster", "harvest", "mutiny", "reform", "alliance"};
    std::string eventId = eventIds[rng() % eventIds.size()];

    std::vector<std::string> aliveIds;
    for (const auto& [fid, f] : factions_) if (f.alive) aliveIds.push_back(fid);
    if (aliveIds.empty()) return "";
    std::string targetId = aliveIds[rng() % aliveIds.size()];
    auto target = findFaction(targetId);

    if (eventId == "disaster") {
        for (auto& city : cities_) {
            if (city.faction == targetId) {
                city.durability = static_cast<int>(city.durability * 0.8);
                addLog("【天灾】" + target->name + "国的" + city.name + "耐久-20%！");
                break;
            }
        }
    } else if (eventId == "harvest") {
        target->troops = static_cast<int>(target->troops * 1.3);
        addLog("【丰收】" + target->name + "国兵力+30%！");
    } else if (eventId == "mutiny") {
        if (!target->generals.empty()) {
            General g = target->generals.back();
            target->generals.pop_back();
            addLog("【兵变】" + target->name + "国的" + g.name + "叛变！");
        }
    } else if (eventId == "reform") {
        target->buffs.push_back({BuffType::All, 0.20, 300.0, "变法"});
        addLog("【变法】" + target->name + "国全属性+20%，持续5分钟！");
    } else if (eventId == "alliance") {
        addLog("【合纵】天下大势，合纵连横！");
    }
    return eventId;
}

City* Game::findCity(const std::string& id) {
    for (auto& c : cities_) {
        if (c.id == id) return &c;
    }
    return nullptr;
}

Faction* Game::findFaction(const std::string& id) {
    auto it = factions_.find(id);
    return it != factions_.end() ? &it->second : nullptr;
}

std::string Game::formatTime() const {
    int minutes = static_cast<int>(elapsedTime_ / 60);
    int seconds = static_cast<int>(elapsedTime_) % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << seconds;
    return oss.str();
}

} // namespace lieguo
