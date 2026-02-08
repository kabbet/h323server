#!/bin/bash

# ============================================================================
# Boost.Test 单元测试运行脚本
# ============================================================================

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${CYAN}========================================${NC}"
}

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$SCRIPT_DIR/build"
TEST_EXECUTABLE="$BUILD_DIR/user_service_tests"

# ============================================================================
# 检查构建
# ============================================================================

if [ ! -d "$BUILD_DIR" ]; then
    print_error "构建目录不存在，请先运行 ./build.sh"
    exit 1
fi

if [ ! -f "$TEST_EXECUTABLE" ]; then
    print_error "测试可执行文件不存在，请先运行 ./build.sh"
    exit 1
fi

# ============================================================================
# 显示菜单
# ============================================================================

show_menu() {
    echo ""
    print_header "Boost.Test 运行选项"
    echo ""
    echo "1) 运行所有测试 (详细输出)"
    echo "2) 运行所有测试 (简洁输出)"
    echo "3) 运行特定测试套件"
    echo "4) 使用 CTest 运行"
    echo "5) 生成 XML 报告"
    echo "6) 生成 HTML 报告"
    echo "7) 列出所有测试用例"
    echo "8) 退出"
    echo ""
}

# ============================================================================
# 测试运行函数
# ============================================================================

run_all_tests_verbose() {
    print_header "运行所有测试 (详细模式)"
    "$TEST_EXECUTABLE" \
        --log_level=all \
        --report_level=detailed \
        --color_output=yes
}

run_all_tests_brief() {
    print_header "运行所有测试 (简洁模式)"
    "$TEST_EXECUTABLE" \
        --log_level=test_suite \
        --report_level=short \
        --color_output=yes
}

run_specific_suite() {
    print_info "可用的测试套件："
    echo "  - PasswordVerificationTests"
    echo "  - AuthenticateUserTests"
    echo "  - CreateUserTokenTests"
    echo "  - ValidateTokenTests"
    echo "  - GetUserInfoTests"
    echo "  - IntegrationTests"
    echo ""
    read -p "请输入测试套件名称: " suite_name
    
    print_header "运行测试套件: $suite_name"
    "$TEST_EXECUTABLE" \
        --run_test="$suite_name" \
        --log_level=all \
        --report_level=detailed \
        --color_output=yes
}

run_ctest() {
    print_header "使用 CTest 运行测试"
    cd "$BUILD_DIR"
    ctest --output-on-failure --verbose
    cd "$SCRIPT_DIR"
}

generate_xml_report() {
    print_header "生成 XML 报告"
    REPORT_FILE="$SCRIPT_DIR/test_report.xml"
    "$TEST_EXECUTABLE" \
        --log_level=all \
        --log_format=XML \
        --log_sink="$REPORT_FILE" \
        --report_level=detailed
    print_success "XML 报告已生成: $REPORT_FILE"
}

generate_html_report() {
    print_header "生成 HTML 报告"
    
    # 先生成 XML
    XML_FILE="$SCRIPT_DIR/test_report.xml"
    "$TEST_EXECUTABLE" \
        --log_level=all \
        --log_format=XML \
        --log_sink="$XML_FILE" \
        --report_level=detailed
    
    # 转换为 HTML (需要 xsltproc)
    if command -v xsltproc &> /dev/null; then
        HTML_FILE="$SCRIPT_DIR/test_report.html"
        # 这里需要 XSLT 样式表，简化版本
        print_info "生成基本 HTML 报告..."
        
        cat > "$HTML_FILE" << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Test Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .success { color: green; }
        .failure { color: red; }
        pre { background: #f4f4f4; padding: 10px; border-radius: 5px; }
    </style>
</head>
<body>
    <h1>Boost.Test Report</h1>
    <p>查看详细的 XML 报告: <a href="test_report.xml">test_report.xml</a></p>
    <pre>
EOF
        "$TEST_EXECUTABLE" --log_level=all --report_level=detailed >> "$HTML_FILE"
        
        cat >> "$HTML_FILE" << 'EOF'
    </pre>
</body>
</html>
EOF
        print_success "HTML 报告已生成: $HTML_FILE"
    else
        print_error "xsltproc 未安装，无法生成 HTML 报告"
        print_info "XML 报告已生成: $XML_FILE"
    fi
}

list_all_tests() {
    print_header "所有测试用例"
    "$TEST_EXECUTABLE" --list_content
}

# ============================================================================
# 主循环
# ============================================================================

while true; do
    show_menu
    read -p "请选择 (1-8): " choice
    
    case $choice in
        1)
            run_all_tests_verbose
            ;;
        2)
            run_all_tests_brief
            ;;
        3)
            run_specific_suite
            ;;
        4)
            run_ctest
            ;;
        5)
            generate_xml_report
            ;;
        6)
            generate_html_report
            ;;
        7)
            list_all_tests
            ;;
        8)
            print_info "退出"
            exit 0
            ;;
        *)
            print_error "无效选择，请重试"
            ;;
    esac
    
    echo ""
    read -p "按 Enter 键继续..."
done
