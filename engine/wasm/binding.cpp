// Emscripten embind 绑定层 - 将 C++ 游戏引擎暴露给 JavaScript
#include "game.h"
#include <emscripten/bind.h>
#include <emscripten/val.h>
using namespace lieguo;
using namespace emscripten;

val factionsToJS(const Game& game) {
    val obj = val::object();
    for (const auto& [id, f] : game.getFactions()) {
        val fo = val::object();
        fo.set("id", f.id); fo.set("name", f.name); fo.set("color", f.color);
        fo.set("trait", f.trait); fo.set("traitDesc", f.traitDesc); fo.set("style", f.style);
        fo.set("troops", f.troops); fo.set("archers", f.archers); fo.set("cavalry", f.cavalry);
        fo.set("playerCount", f.playerCount); fo.set("alive", f.alive);
        fo.set("citiesOwned", f.citiesOwned); fo.set("totalContribution", f.totalContribution);
        val ga = val::array();
        for (size_t i = 0; i < f.generals.size(); i++) {
            const auto& g = f.generals[i];
            val go = val::object();
            go.set("id", g.id); go.set("name", g.name);
            go.set("quality", g.quality == GeneralQuality::Orange ? "orange" : g.quality == GeneralQuality::Purple ? "purple" : "blue");
            go.set("faction", g.faction); go.set("passive", g.passive);
            go.set("skill", g.skill); go.set("skillDesc", g.skillDesc);
            go.set("level", g.level); go.set("cooldown", g.cooldown);
            ga.set(i, go);
        }
        fo.set("generals", ga);
        val ba = val::array();
        for (size_t i = 0; i < f.buffs.size(); i++) {
            const auto& b = f.buffs[i];
            val bo = val::object();
            bo.set("type", b.type == BuffType::Attack ? "attack" : b.type == BuffType::Defense ? "defense" : "all");
            bo.set("value", b.value); bo.set("duration", b.duration); bo.set("source", b.source);
            ba.set(i, bo);
        }
        fo.set("buffs", ba);
        obj.set(id, fo);
    }
    return obj;
}

val citiesToJS(const Game& game) {
    val arr = val::array();
    const auto& cities = game.getCities();
    for (size_t i = 0; i < cities.size(); i++) {
        const auto& c = cities[i];
        val co = val::object();
        co.set("id", c.id); co.set("name", c.name);
        co.set("type", c.type == CityType::Capital ? "capital" : c.type == CityType::Fortress ? "fortress" : "normal");
        co.set("faction", c.faction); co.set("x", c.x); co.set("y", c.y);
        co.set("durability", c.durability); co.set("maxDurability", c.maxDurability);
        co.set("underAttack", c.underAttack); co.set("attacker", c.attacker);
        val na = val::array();
        for (size_t j = 0; j < c.neighbors.size(); j++) na.set(j, c.neighbors[j]);
        co.set("neighbors", na);
        arr.set(i, co);
    }
    return arr;
}

val alliancesToJS(const Game& game) {
    val arr = val::array();
    const auto& alliances = game.getAlliances();
    for (size_t i = 0; i < alliances.size(); i++) {
        const auto& a = alliances[i];
        val ao = val::object();
        val ma = val::array();
        for (size_t j = 0; j < a.members.size(); j++) ma.set(j, a.members[j]);
        ao.set("members", ma); ao.set("remaining", a.remaining); ao.set("totalDuration", a.totalDuration);
        arr.set(i, ao);
    }
    return arr;
}

val battleLogsToJS(const Game& game) {
    val arr = val::array();
    const auto& logs = game.getBattleLogs();
    for (size_t i = 0; i < logs.size(); i++) {
        val lo = val::object();
        lo.set("time", logs[i].time); lo.set("msg", logs[i].msg);
        arr.set(i, lo);
    }
    return arr;
}

val giftsToJS() {
    val arr = val::array();
    auto gifts = getGifts();
    for (size_t i = 0; i < gifts.size(); i++) {
        const auto& g = gifts[i];
        val go = val::object();
        go.set("id", g.id); go.set("name", g.name); go.set("price", g.price);
        go.set("troops", g.troops); go.set("buff", g.buff); go.set("duration", g.duration);
        go.set("quality", g.quality); go.set("damage", g.damage); go.set("desc", g.desc);
        std::string e;
        switch (g.effect) {
            case GiftEffect::Join: e = "join"; break;
            case GiftEffect::Archer: e = "archer"; break;
            case GiftEffect::Buff: e = "buff"; break;
            case GiftEffect::General: e = "general"; break;
            case GiftEffect::Damage: e = "damage"; break;
            case GiftEffect::Capture: e = "capture"; break;
            case GiftEffect::Sabotage: e = "sabotage"; break;
        }
        go.set("effect", e);
        arr.set(i, go);
    }
    return arr;
}

val sendGiftJS(Game& game, const std::string& giftId, const std::string& targetCityId) {
    auto result = game.sendGift(giftId, targetCityId);
    val obj = val::object();
    obj.set("success", result.success); obj.set("msg", result.msg);
    return obj;
}

EMSCRIPTEN_BINDINGS(lieguo_module) {
    enum_<GameState>("GameState")
        .value("Idle", GameState::Idle).value("Playing", GameState::Playing)
        .value("Paused", GameState::Paused).value("Ended", GameState::Ended);
    class_<Game>("Game")
        .constructor<>()
        .function("init", &Game::init).function("start", &Game::start)
        .function("pause", &Game::pause).function("resume", &Game::resume)
        .function("stop", &Game::stop).function("tick", &Game::tick)
        .function("joinFaction", &Game::joinFaction).function("like", &Game::like)
        .function("comment", &Game::comment).function("sendGift", &sendGiftJS)
        .function("createAlliance", &Game::createAlliance).function("breakAlliance", &Game::breakAlliance)
        .function("isAllied", &Game::isAllied).function("triggerRandomEvent", &Game::triggerRandomEvent)
        .function("getState", &Game::getState).function("getElapsedTime", &Game::getElapsedTime)
        .function("getPlayerFaction", &Game::getPlayerFaction).function("getHuFu", &Game::getHuFu)
        .function("getWinner", &Game::getWinner).function("formatTime", &Game::formatTime)
        .function("getFactionsJS", &factionsToJS).function("getCitiesJS", &citiesToJS)
        .function("getAlliancesJS", &alliancesToJS).function("getBattleLogsJS", &battleLogsToJS);
    function("getGiftsJS", &giftsToJS);
    register_vector<std::string>("VectorString");
}
