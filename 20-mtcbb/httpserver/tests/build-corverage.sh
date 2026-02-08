#!/bin/bash

# ============================================================================
# 启用代码覆盖率的构建脚本
# ============================================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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
    echo -e "${RED}[WARNING]${NC} $1"
}

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

BUILD_DIR="build_coverage"
JOBS=$(nproc)

print_info "=========================================="
print_info "构建启用代码覆盖率的测试版本"
print_info "=========================================="
echo ""

# ============================================================================
# 检查依赖
# ============================================================================

print_info "检查依赖工具..."

if ! command -v gcov &> /dev/null; then
    print_warning "gcov 未安装"
fi

if ! command -v lcov &> /dev/null; then
    print_warning "lcov 未安装（可选，用于生成 HTML 报告）"
    print_info "安装: sudo apt-get install lcov"
fi

# ============================================================================
# 清理旧构建
# ============================================================================

if [ -d "$BUILD_DIR" ]; then
    print_info "清理旧的覆盖率构建..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# ============================================================================
# 配置 CMake（启用覆盖率）
# ============================================================================

print_info "配置 CMake（启用代码覆盖率）..."

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE=Debug
    -DCMAKE_CXX_FLAGS="--coverage -g -O0"
    -DCMAKE_EXE_LINKER_FLAGS="--coverage"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

if cmake "${CMAKE_ARGS[@]}" ..; then
    print_success "CMake 配置成功"
else
    print_error "CMake 配置失败"
    exit 1
fi

echo ""

# ============================================================================
# 编译
# ============================================================================

print_info "编译测试程序（使用 $JOBS 个并行任务）..."

if cmake --build . -j"$JOBS"; then
    print_success "编译成功"
else
    print_error "编译失败"
    exit 1
fi

echo ""

# ============================================================================
# 运行测试
# ============================================================================

print_info "运行测试以生成覆盖率数据..."

if [ -f "user_service_tests" ]; then
    ./user_service_tests --log_level=test_suite --report_level=short
    
    if [ $? -eq 0 ]; then
        print_success "测试通过"
    else
        print_warning "部分测试失败"
    fi
else
    print_error "测试可执行文件不存在"
    exit 1
fi

echo ""

# ============================================================================
# 验证覆盖率文件
# ============================================================================

print_info "验证覆盖率数据文件..."

GCDA_COUNT=$(find . -name "*.gcda" | wc -l)
GCNO_COUNT=$(find . -name "*.gcno" | wc -l)

if [ "$GCDA_COUNT" -eq 0 ]; then
    print_error "未生成 .gcda 文件（运行时数据）"
    exit 1
else
    print_success "找到 $GCDA_COUNT 个 .gcda 文件"
fi

if [ "$GCNO_COUNT" -eq 0 ]; then
    print_error "未生成 .gcno 文件（编译时数据）"
    exit 1
else
    print_success "找到 $GCNO_COUNT 个 .gcno 文件"
fi

echo ""

# ============================================================================
# 生成覆盖率报告
# ============================================================================

print_info "生成覆盖率报告..."

# 返回到测试根目录
cd "$SCRIPT_DIR"

# 方法 1: 使用 gcov（简单）
print_info "使用 gcov 生成基本报告..."

COVERAGE_DIR="coverage_output"
rm -rf "$COVERAGE_DIR"
mkdir -p "$COVERAGE_DIR"

# 找到对象文件目录
OBJECT_DIR=$(find "$BUILD_DIR" -name "UserService.cpp.gcno" -printf '%h\n' | head -1)

if [ -n "$OBJECT_DIR" ]; then
    cd "$OBJECT_DIR"
    
    # 对每个 .gcno 文件运行 gcov
    for gcno_file in *.gcno; do
        if [ -f "${gcno_file%.gcno}.gcda" ]; then
            gcov "${gcno_file%.gcno}" -o . 2>/dev/null > /dev/null
        fi
    done
    
    # 移动生成的 .gcov 文件
    cd "$SCRIPT_DIR"
    find "$OBJECT_DIR" -name "*.gcov" -exec mv {} "$COVERAGE_DIR/" \;
    
    print_success "gcov 报告已生成: $COVERAGE_DIR/"
    
    # 显示 UserService.cpp 的覆盖率
    if [ -f "$COVERAGE_DIR/UserService.cpp.gcov" ]; then
        echo ""
        print_info "UserService.cpp 覆盖率概览:"
        echo "----------------------------------------"
        
        TOTAL=$(grep -E "^\s*[0-9]+" "$COVERAGE_DIR/UserService.cpp.gcov" | wc -l)
        COVERED=$(grep -E "^\s*[1-9][0-9]*:" "$COVERAGE_DIR/UserService.cpp.gcov" | wc -l)
        
        if [ "$TOTAL" -gt 0 ]; then
            PERCENT=$(awk "BEGIN {printf \"%.2f\", ($COVERED / $TOTAL) * 100}")
            echo "  已覆盖行数: $COVERED / $TOTAL"
            echo "  覆盖率: ${PERCENT}%"
        fi
        
        echo "----------------------------------------"
    fi
fi

echo ""

# 方法 2: 使用 lcov（HTML 报告）
if command -v lcov &> /dev/null; then
    print_info "使用 lcov 生成 HTML 报告..."
    
    HTML_DIR="coverage_html"
    rm -rf "$HTML_DIR"
    
    # 收集覆盖率数据
    lcov --capture \
         --directory "$BUILD_DIR" \
         --output-file coverage.info \
         --rc lcov_branch_coverage=1 \
         2>/dev/null
    
    # 过滤不需要的文件
    lcov --remove coverage.info \
         '/usr/*' \
         '*/test_*' \
         '*/mocks/*' \
         '*/10-common/*' \
         --output-file coverage_filtered.info \
         2>/dev/null
    
    # 生成 HTML
    genhtml coverage_filtered.info \
            --output-directory "$HTML_DIR" \
            --title "UserService Test Coverage" \
            --legend \
            --rc genhtml_branch_coverage=1 \
            2>/dev/null
    
    if [ -f "$HTML_DIR/index.html" ]; then
        print_success "HTML 报告已生成: $HTML_DIR/index.html"
        
        # 显示总体覆盖率
        if command -v lcov &> /dev/null; then
            echo ""
            lcov --summary coverage_filtered.info 2>/dev/null | grep -A 3 "lines"
        fi
    fi
else
    print_warning "lcov 未安装，跳过 HTML 报告生成"
    print_info "安装: sudo apt-get install lcov"
fi

echo ""

# ============================================================================
# 完成
# ============================================================================

print_success "=========================================="
print_success "代码覆盖率分析完成！"
print_success "=========================================="
echo ""
print_info "报告位置:"
echo "  gcov 文本: $COVERAGE_DIR/"
if [ -d "$HTML_DIR" ]; then
    echo "  HTML 报告: $HTML_DIR/index.html"
fi
echo ""
print_info "查看 UserService.cpp 详细覆盖:"
echo "  less $COVERAGE_DIR/UserService.cpp.gcov"
echo ""

if [ -f "$HTML_DIR/index.html" ]; then
    echo "在浏览器中打开 HTML 报告:"
    echo "  xdg-open $HTML_DIR/index.html"
    echo "  # 或"
    echo "  firefox $HTML_DIR/index.html"
fi
