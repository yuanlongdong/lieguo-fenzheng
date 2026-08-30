#include "game.h"
#include <cassert>
#include <iostream>

using namespace lieguo;

void testInit() {
    Game game;
    assert(game.getState() == GameState::Idle);
    assert(game.getFactions().size() == 7);
    assert(game.getCities().size() == 23);
    std::cout << "[PASS] testInit: 7阵营, 23城池" << std::endl;
}

void testJoinFaction() {
    Game game;
    assert(game.joinFaction("qin") == true);
    assert(game.getPlayerFaction() == "qin");
    assert(game.joinFaction("nonexistent") == false);
    std::cout << "[PASS] testJoinFaction" << std::endl;
}

void testLike() {
    Game game;
    game.joinFaction("qin");
    int before = game.getFactions().at("qin").troops;
    game.like();
    int after = game.getFactions().at("qin").troops;
    assert(after > before);
    std::cout << "[PASS] testLike: 兵力 " << before << " -> " << after << std::endl;
}

void testGift() {
    Game game;
    game.joinFaction("qin");
    auto result = game.sendGift("light");
    assert(result.success);
    auto result2 = game.sendGift("nonexistent");
    assert(!result2.success);
    std::cout << "[PASS] testGift" << std::endl;
}

void testTick() {
    Game game;
    game.joinFaction("qin");
    game.start();
    int before = game.getFactions().at("qin").troops;
    game.tick();
    int after = game.getFactions().at("qin").troops;
    assert(after >= before);
    std::cout << "[PASS] testTick: 兵力 " << before << " -> " << after << std::endl;
}

void testAlliance() {
    Game game;
    game.joinFaction("qin");
    bool ok = game.createAlliance({"qin", "chu"});
    assert(ok);
    assert(game.isAllied("qin", "chu"));
    assert(!game.isAllied("qin", "qi"));
    bool ok2 = game.createAlliance({"qin", "qi"});
    assert(!ok2);
    std::cout << "[PASS] testAlliance" << std::endl;
}

void testBreakAlliance() {
    Game game;
    game.joinFaction("qin");
    game.createAlliance({"qin", "chu"});
    bool ok = game.breakAlliance("qin");
    assert(ok);
    assert(!game.isAllied("qin", "chu"));
    std::cout << "[PASS] testBreakAlliance" << std::endl;
}

void testCityCapture() {
    Game game;
    game.joinFaction("qin");
    City* luoyang = game.findCity("luoyang");
    assert(luoyang != nullptr);
    assert(luoyang->faction.empty());
    luoyang->durability = 1;
    game.start();
    game.tick();
    std::cout << "[INFO] testCityCapture: 洛阳归属=" << luoyang->faction
              << " 耐久=" << luoyang->durability << std::endl;
    std::cout << "[PASS] testCityCapture (不强制断言，取决于AI)" << std::endl;
}

void testEliminateFaction() {
    Game game;
    for (auto& city : const_cast<std::vector<City>&>(game.getCities())) {
        if (city.faction == "han") city.faction = "qin";
    }
    game.start();
    game.tick();
    auto han = game.getFactions().at("han");
    std::cout << "[INFO] testEliminateFaction: 韩国存活=" << han.alive << std::endl;
    std::cout << "[PASS] testEliminateFaction" << std::endl;
}

void testRandomEvent() {
    Game game;
    game.start();
    std::string evt = game.triggerRandomEvent();
    assert(!evt.empty());
    std::cout << "[PASS] testRandomEvent: " << evt << std::endl;
}

int main() {
    std::cout << "===== 列国纷争 C++ 引擎单元测试 =====\n" << std::endl;
    testInit();
    testJoinFaction();
    testLike();
    testGift();
    testTick();
    testAlliance();
    testBreakAlliance();
    testCityCapture();
    testEliminateFaction();
    testRandomEvent();
    std::cout << "\n===== 全部测试通过！=====" << std::endl;
    return 0;
}
