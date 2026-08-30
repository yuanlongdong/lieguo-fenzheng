# 列国纷争 - C++ 游戏引擎

> 战国七雄弹幕互动对抗游戏的 C++ 底层核心引擎

## 概述

本目录包含《列国纷争》游戏的 C++ 核心引擎实现。游戏逻辑、战斗系统、数值计算、外交系统全部用 C++17 实现，可独立编译运行，也可作为库接入前端（Qt/WASM/服务端）。

## 架构

```
engine/cpp/
├── CMakeLists.txt          # CMake 构建配置
├── README.md               # 本文件
├── include/
│   ├── types.h             # 核心数据结构（阵营/城池/武将/礼物/同盟/Buff）
│   ├── config.h            # 配置接口声明
│   └── game.h              # 游戏引擎类声明
├── src/
│   ├── config.cpp          # 静态数据（七雄/23城/12武将/10礼物）
│   ├── game.cpp            # 游戏核心逻辑（战斗/攻城/武将/外交/礼物/事件）
│   └── main.cpp            # 控制台 Demo 入口
└── tests/
    └── test_game.cpp       # 单元测试（10项）
```

## 核心模块

### Game 类（game.h）
游戏引擎主类，负责：
- **生命周期**：init/start/pause/resume/stop
- **主循环**：tick() 单步推进，产兵→攻城→武将→Buff→同盟→耐久→胜负判定
- **玩家操作**：joinFaction/like/comment/sendGift
- **外交系统**：createAlliance/breakAlliance/isAllied
- **随机事件**：triggerRandomEvent（天灾/丰收/兵变/变法/合纵）
- **线程安全**：所有公共方法加 mutex，可多线程调用

### 战斗系统
- **攻击力计算**：基础兵力 + 弓兵×1.25 + 骑兵×1.1，叠加阵营特性/武将被动/Buff
- **防御力计算**：兵力/城数 × 城池防御系数，叠加阵营特性/武将被动/Buff
- **攻城逻辑**：攻击>防御时削减耐久，攻城方每秒损耗1%兵力
- **城池占领**：耐久归零易主，占领后耐久恢复50%
- **灭国判定**：失去所有城池即灭亡，城池变中立

### 武将系统
- 12名武将，橙/紫/蓝三品质
- 每名武将有被动光环 + 主动技能
- 技能自动释放，冷却60秒（可被孙膑/申不害减免）
- 重复召唤升级（最高3级），满3员替换最弱

### 礼物系统
10种礼物，从1抖币灯牌到3万抖币嘉年华：
- 灯牌/仙女棒：产兵
- 能力药丸/魔法镜：阵营Buff
- 派对话筒/神秘空投/跑车：召唤武将
- 爱的爆炸：直接伤害城池
- 嘉年华：直接夺取城池（耐久<50%非首都）
- 离间计：50%拆散敌方同盟

### 合纵连横
- 20分钟后解锁结盟
- 同盟持续10分钟，2-3国
- 背盟惩罚：全属性-20%持续5分钟
- 离间计可拆散敌方同盟

## 编译运行

### 依赖
- C++17 编译器（GCC 7+ / Clang 5+ / MSVC 2017+）
- CMake 3.14+
- pthread（Linux/Mac）

### 编译
```bash
cd engine/cpp
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON
make -j$(nproc)
```

### 运行 Demo
```bash
./lieguo_demo
```

Demo 支持命令交互：
```
status     - 查看游戏状态
factions   - 查看七雄状态
cities     - 查看城池状态
like       - 点赞 (+1兵)
comment    - 发送弹幕
gift <id>  - 送礼物
ally       - 尝试结盟
break      - 背盟
event      - 触发随机事件
pause      - 暂停
resume     - 继续
logs       - 查看战报
help       - 帮助
quit       - 退出
```

### 运行测试
```bash
./test_game
```

## 作为库使用

```cpp
#include "game.h"
using namespace lieguo;

Game game;
game.joinFaction("qin");  // 加入秦国
game.start();             // 开始游戏

// 游戏循环
while (game.getState() == GameState::Playing) {
    game.tick();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// 玩家操作
game.like();
game.sendGift("bomb", "luoyang");  // 对洛阳使用爱的爆炸
game.createAlliance({"qin", "chu"});
```

## 接入前端

### 方案一：Qt 桌面端
- 用 Qt Widgets/QML 做 UI 层
- Game 类作为核心逻辑，通过信号槽更新 UI
- 参考 `main.cpp` 的命令交互模式

### 方案二：WebAssembly（浏览器端）
- 用 Emscripten 编译为 WASM
- 前端 JS 通过 WASM 接口调用 Game 方法
- Canvas 渲染地图，HTML/CSS 做面板

### 方案三：服务端
- Game 类运行在服务端，管理一局游戏
- 前端通过 WebSocket 发送操作（点赞/礼物/弹幕）
- 服务端广播游戏状态给所有观众

## 数值设计

| 参数 | 值 |
|------|-----|
| 初始兵力 | 500/国 |
| 首都耐久 | 50000 |
| 重镇耐久 | 30000 |
| 普通城耐久 | 20000 |
| 首都产兵 | 5/秒 |
| 重镇产兵 | 4/秒 |
| 普通城产兵 | 2/秒 |
| 攻城损耗 | 1%兵力/秒 |
| 同盟持续 | 10分钟 |
| 合纵解锁 | 20分钟 |
| 武将冷却 | 60秒 |

## 线程安全

Game 类所有修改状态的公共方法都加了 `std::mutex` 保护：
- 游戏循环线程调用 tick()
- 主线程/网络线程调用玩家操作（like/sendGift 等）
- 两者可安全并发

## 许可证

MIT License
