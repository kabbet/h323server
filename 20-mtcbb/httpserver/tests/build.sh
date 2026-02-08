#!/bin/bash

# ============================================================================
# Boost.Test 单元测试构建脚本
# ============================================================================

set -e  # 遇到错误立即退出

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 函数：打印带颜色的信息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# ============================================================================
# 配置选项
# ============================================================================

BUILD_TYPE="${1:-Debug}"  # 默认 Debug 模式
BUILD_DIR="build"
JOBS=$(nproc)  # 使用所有可用 CPU 核心

print_info "构建配置:"
echo "  - Build Type: $BUILD_TYPE"
echo "  - Build Directory: $BUILD_DIR"
echo "  - Parallel Jobs: $JOBS"
echo ""

# ============================================================================
# 检查依赖
# ============================================================================

print_info "检查依赖..."

# 检查 CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake 未安装，请先安装 CMake"
    exit 1
fi

# 检查 Boost
if ! ldconfig -p | grep -q libboost_unit_test_framework; then
    print_warning "未检测到 Boost.Test 库，可能需要安装"
    print_info "Ubuntu/Debian: sudo apt-get install libboost-test-dev"
    print_info "CentOS/RHEL: sudo yum install boost-devel"
fi

# 检查 Drogon
if ! ldconfig -p | grep -q libdrogon; then
    print_warning "未检测到 Drogon 库"
    print_info "请确保 Drogon 已安装: https://github.com/drogonframework/drogon"
fi

print_success "依赖检查完成"
echo ""

# ============================================================================
# 清理旧的构建文件（可选）
# ============================================================================

if [ "$2" == "clean" ]; then
    print_info "清理旧的构建文件..."
    rm -rf "$BUILD_DIR"
    print_success "清理完成"
    echo ""
fi

# ============================================================================
# 创建构建目录
# ============================================================================

print_info "创建构建目录..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# ============================================================================
# 配置项目
# ============================================================================

print_info "配置 CMake..."

# 你可能需要根据实际情况修改 PROJECT_INCLUDE_DIR
# 它应该指向你的项目 include 目录
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

# 如果你的项目在特定位置，添加这个参数
# CMAKE_ARGS+=(-DPROJECT_INCLUDE_DIR="/path/to/your/project/include")

if cmake "${CMAKE_ARGS[@]}" ..; then
    print_success "CMake 配置成功"
else
    print_error "CMake 配置失败"
    exit 1
fi

echo ""

# ============================================================================
# 编译项目
# ============================================================================

print_info "编译测试程序 (使用 $JOBS 个并行任务)..."

if cmake --build . -j"$JOBS"; then
    print_success "编译成功"
else
    print_error "编译失败"
    exit 1
fi

echo ""

# ============================================================================
# 完成
# ============================================================================

print_success "构建完成！"
echo ""
echo "可执行文件位置: $BUILD_DIR/bin/user_service_tests"
echo ""
echo "运行测试："
echo "  ./run_tests.sh"
echo ""
echo "或直接运行："
echo "  cd $BUILD_DIR"
echo "  ctest -V"
echo "  # 或"
echo "  ./bin/user_service_tests --log_level=all"
