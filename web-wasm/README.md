# 列国纷争 - WebAssembly 版

> C++ 游戏引擎编译为 WebAssembly，HTML5 前端通过 JS 调用

## 架构

```
web-wasm/
├── index.html              # WASM 版前端入口
├── lieguo.js               # Emscripten 生成的 JS 胶水代码（需编译）
├── lieguo.wasm             # 编译后的 WebAssembly 模块（需编译）
├── css/style.css           # 样式（与原版相同）
└── js/
    ├── config.js           # 游戏配置数据（与原版相同）
    ├── wasm-game.js        # WASM 引擎包装器（核心）
    ├── map.js              # 地图渲染（与原版相同）
    └── ui.js               # UI 管理（与原版相同）
```

## 核心设计

### wasm-game.js 包装器
将 C++ 编译的 WebAssembly 引擎包装为与原 JS 版 Game 类完全相同的接口，使 map.js 和 ui.js 无需修改即可使用。

- 异步加载 WASM 模块并创建 C++ Game 实例
- 每帧从 C++ 引擎同步数据到 JS 属性
- 实现事件系统（on/emit），支持 update/log/cityCaptured/victory
- 实现主循环（setInterval 调用 game.tick()）

### binding.cpp（Emscripten embind 绑定层）
将 C++ Game 类暴露给 JavaScript，提供 JS 友好的数据获取方法（getFactionsJS/getCitiesJS 等），返回纯 JS 对象。

## 编译 WASM

```bash
# 安装 Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# 编译
cd engine/wasm
bash build.sh

# 将编译产物复制到前端目录
cp output/lieguo.js output/lieguo.wasm ../../web-wasm/
```

## 运行

```bash
cd web-wasm
python3 -m http.server 8080
# 浏览器打开 http://localhost:8080
```

## 与纯 JS 版对比

| 特性 | 纯 JS 版 | WASM 版 |
|------|---------|--------|
| 游戏逻辑 | JavaScript | C++ 编译为 WebAssembly |
| 性能 | 一般 | 更高 |
| 加载速度 | 快 | 较慢（需下载 WASM） |
| 代码保护 | 易读 | 编译后难以逆向 |
| 前端渲染 | Canvas | Canvas（相同） |
