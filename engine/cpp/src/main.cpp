#include "game.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

using namespace lieguo;

static std::atomic<bool> g_running{true};

void signalHandler(int) { g_running = false; }

void printHelp() {
    std::cout << "\n===== 命令列表 =====\n"
              << "  status     - 查看游戏状态\n"
              << "  factions   - 查看七雄状态\n"
              << "  cities     - 查看城池状态\n"
              << "  like       - 点赞 (+1兵)\n"
              << "  comment    - 发送弹幕\n"
              << "  gift <id>  - 送礼物 (light/wand/pill/mic/bomb/mirror/drop/car/carnival/sabotage)\n"
              << "  ally       - 尝试结盟\n"
              << "  break      - 背盟\n"
              << "  event      - 触发随机事件\n"
              << "  pause      - 暂停游戏\n"
              << "  resume     - 继续游戏\n"
              << "  logs       - 查看最近战报\n"
              << "  help       - 显示帮助\n"
              << "  quit       - 退出游戏\n"
              << "====================\n" << std::endl;
}

void printStatus(Game& game) {
    std::cout << "\n===== 游戏状态 =====\n"
              << "时间: " << game.formatTime() << "\n"
              << "状态: ";
    switch (game.getState()) {
        case GameState::Idle: std::cout << "待机"; break;
        case GameState::Playing: std::cout << "进行中"; break;
        case GameState::Paused: std::cout << "已暂停"; break;
        case GameState::Ended: std::cout << "已结束"; break;
    }
    std::cout << "\n玩家阵营: " << (game.getPlayerFaction().empty() ? "未加入" : game.getPlayerFaction())
              << "\n虎符: " << game.getHuFu()
              << "\n同盟数: " << game.getAlliances().size()
              << "\n";
    if (!game.getWinner().empty()) {
        std::cout << "★ 胜者: " << game.getWinner() << "国 ★\n";
    }
    std::cout << "====================\n" << std::endl;
}

void printFactions(Game& game) {
    std::cout << "\n===== 七雄争霸 =====\n";
    for (const auto& [id, f] : game.getFactions()) {
        std::cout << (f.alive ? "  " : "✗ ")
                  << f.name << "国 [" << f.trait << "] "
                  << "城:" << f.citiesOwned
                  << " 兵:" << f.troops
                  << " 弓:" << f.archers
                  << " 骑:" << f.cavalry
                  << " 将:" << f.generals.size();
        if (!f.buffs.empty()) {
            std::cout << " Buff:";
            for (const auto& b : f.buffs) std::cout << " " << b.source;
        }
        std::cout << "\n";
    }
    std::cout << "====================\n" << std::endl;
}

void printCities(Game& game) {
    std::cout << "\n===== 城池列表 =====\n";
    for (const auto& c : game.getCities()) {
        std::string owner = c.faction.empty() ? "中立" : c.faction;
        std::string type = c.type == CityType::Capital ? "首都" :
                          c.type == CityType::Fortress ? "重镇" : "城池";
        std::cout << "  " << c.name << "(" << type << ") "
                  << "归属:" << owner
                  << " 耐久:" << c.durability << "/" << c.maxDurability;
        if (c.underAttack) std::cout << " ⚔被" << c.attacker << "攻击";
        std::cout << "\n";
    }
    std::cout << "====================\n" << std::endl;
}

void printLogs(Game& game) {
    std::cout << "\n===== 最近战报 =====\n";
    int count = 0;
    for (const auto& log : game.getBattleLogs()) {
        int min = static_cast<int>(log.time / 60);
        int sec = static_cast<int>(log.time) % 60;
        std::cout << "  [" << min << ":" << (sec < 10 ? "0" : "") << sec << "] " << log.msg << "\n";
        if (++count >= 15) break;
    }
    std::cout << "====================\n" << std::endl;
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "========================================\n"
              << "    列国纷争 - C++ 游戏引擎 Demo\n"
              << "    战国七雄 · 弹幕互动 · 合纵连横\n"
              << "========================================\n" << std::endl;

    Game game;
    game.onLog([](const std::string&) {});
    game.joinFaction("qin");
    std::cout << ">> 已自动加入秦国阵营\n" << std::endl;

    game.start();
    std::cout << ">> 游戏开始！输入 help 查看命令\n" << std::endl;

    std::thread gameThread([&]() {
        while (g_running) {
            if (game.getState() == GameState::Playing) {
                game.tick();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    std::string cmd;
    while (g_running && game.getState() != GameState::Ended) {
        std::cout << "\n> " << std::flush;
        if (!std::getline(std::cin, cmd)) break;
        if (cmd.empty()) continue;

        if (cmd == "quit" || cmd == "exit") {
            g_running = false; break;
        } else if (cmd == "help") {
            printHelp();
        } else if (cmd == "status") {
            printStatus(game);
        } else if (cmd == "factions") {
            printFactions(game);
        } else if (cmd == "cities") {
            printCities(game);
        } else if (cmd == "like") {
            game.like();
            std::cout << ">> 点赞成功！+1兵力" << std::endl;
        } else if (cmd.rfind("comment", 0) == 0) {
            std::string text = cmd.size() > 8 ? cmd.substr(8) : "666";
            game.comment(text);
            std::cout << ">> 弹幕已发送: " << text << std::endl;
        } else if (cmd.rfind("gift", 0) == 0) {
            std::string giftId = cmd.size() > 5 ? cmd.substr(5) : "";
            if (giftId.empty()) {
                std::cout << "用法: gift <light|wand|pill|mic|bomb|mirror|drop|car|carnival|sabotage>" << std::endl;
            } else {
                auto result = game.sendGift(giftId);
                std::cout << ">> " << (result.success ? "礼物送出成功！" : "失败: " + result.msg) << std::endl;
            }
        } else if (cmd == "ally") {
            std::vector<std::string> alive;
            for (const auto& [id, f] : game.getFactions()) {
                if (f.alive && id != game.getPlayerFaction()) alive.push_back(id);
            }
            if (!alive.empty()) {
                bool ok = game.createAlliance({game.getPlayerFaction(), alive[0]});
                std::cout << ">> " << (ok ? "结盟成功！" : "结盟失败（可能已有同盟）") << std::endl;
            }
        } else if (cmd == "break") {
            bool ok = game.breakAlliance(game.getPlayerFaction());
            std::cout << ">> " << (ok ? "背盟成功！全属性-20%持续5分钟" : "背盟失败（没有同盟）") << std::endl;
        } else if (cmd == "event") {
            std::string evt = game.triggerRandomEvent();
            std::cout << ">> 触发事件: " << evt << std::endl;
        } else if (cmd == "pause") {
            game.pause();
            std::cout << ">> 游戏已暂停" << std::endl;
        } else if (cmd == "resume") {
            game.resume();
            std::cout << ">> 游戏继续" << std::endl;
        } else if (cmd == "logs") {
            printLogs(game);
        } else {
            std::cout << ">> 未知命令，输入 help 查看帮助" << std::endl;
        }
    }

    g_running = false;
    if (gameThread.joinable()) gameThread.join();

    std::cout << "\n========================================\n"
              << "    游戏结束！最终状态:\n"
              << "========================================\n";
    printStatus(game);
    printFactions(game);
    printLogs(game);
    return 0;
}
