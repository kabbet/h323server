#!/bin/bash
set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'  # No Color

# ============ 工具函数 ============
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

print_step "=========================================="
print_step "开始编译 ${version_type} 版本"
print_step "=========================================="


cmake -B build -G Ninja \
        -DCMAKE_VERBOSE_MAKEFILE=ON \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -DCMAKE_BUILD_TYPE="Debug" \
        ../.. > /dev/null 2>&1
cp -vr build/compile_commands.json ../..
print_step "compile_commands 已生成"
rm -rf build
