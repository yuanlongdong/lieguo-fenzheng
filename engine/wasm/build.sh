#!/bin/bash
# 列国纷争 WASM 构建脚本
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP_DIR="$SCRIPT_DIR/../cpp"
OUTPUT_DIR="$SCRIPT_DIR/output"

if ! command -v emcc &> /dev/null; then
    echo "错误: 未找到 emcc，请先安装并激活 Emscripten"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
echo "===== 编译列国纷争 C++ 引擎为 WebAssembly ====="

emcc \
    -std=c++17 \
    -O1 \
    --bind \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="createLieGuoModule" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MAXIMUM_MEMORY=256MB \
    -s NO_FILESYSTEM=1 \
    -s MALLOC=emmalloc \
    -s SUPPORT_ERRNO=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -s ENVIRONMENT='web' \
    -I"$CPP_DIR/include" \
    "$CPP_DIR/src/config.cpp" \
    "$CPP_DIR/src/game.cpp" \
    "$SCRIPT_DIR/binding.cpp" \
    -o "$OUTPUT_DIR/lieguo.js"

echo "===== 编译完成 ====="
ls -lh "$OUTPUT_DIR/"
