#!/bin/bash

################################################################################
# H.323 库完整编译安装脚本
# 集成修复方案：解决 H323PluginCodecManager 链接错误
# 编译 ptlib, h323plus, gnugk 并安装到指定目录
# 
# 项目结构:
# project/
# ├── 10-common/
# │   ├── include/
# │   └── lib/releaselib/linux64/
# │       ├── debug/
# │       └── release/
# ├── 20-mtcbb/
# └── b0-thirdparty/
#     ├── ptlib/
#     ├── h323plus/
#     └── gnugk/
################################################################################

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 打印函数
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

print_success() {
    echo -e "${CYAN}[SUCCESS]${NC} $1"
}

################################################################################
# 配置部分
################################################################################

# 获取脚本所在目录（假设脚本在项目根目录）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}"

# 源码目录
THIRDPARTY_DIR="${PROJECT_ROOT}/b0-thirdparty"
PTLIB_SRC="${THIRDPARTY_DIR}/ptlib"
H323PLUS_SRC="${THIRDPARTY_DIR}/h323plus"
GNUGK_SRC="${THIRDPARTY_DIR}/gnugk"

# 安装目录
COMMON_DIR="${PROJECT_ROOT}/10-common"
INCLUDE_DIR="${COMMON_DIR}/include"
LIB_BASE_DIR="${COMMON_DIR}/lib/releaselib/linux64"
DEBUG_LIB_DIR="${LIB_BASE_DIR}/debug"
RELEASE_LIB_DIR="${LIB_BASE_DIR}/release"

# 构建临时目录
BUILD_DIR="${PROJECT_ROOT}/build_temp_h323"

# 编译参数
JOBS=$(nproc 2>/dev/null || echo 4)  # 使用所有CPU核心，默认4
BUILD_DEBUG=${BUILD_DEBUG:-yes}      # 是否编译debug版本
BUILD_RELEASE=${BUILD_RELEASE:-yes}  # 是否编译release版本

################################################################################
# 函数定义
################################################################################

# 显示使用帮助
show_usage() {
    cat << EOF
用法: $0 [选项]

选项:
    -d, --debug-only      仅编译 debug 版本
    -r, --release-only    仅编译 release 版本
    -j, --jobs N          使用 N 个并行任务编译 (默认: ${JOBS})
    -c, --clean           清理之前的编译
    -h, --help            显示此帮助信息

环境变量:
    BUILD_DEBUG=yes|no    控制是否编译 debug 版本
    BUILD_RELEASE=yes|no  控制是否编译 release 版本

示例:
    $0                    # 编译 debug 和 release 版本
    $0 --debug-only       # 仅编译 debug 版本
    $0 --clean            # 清理后重新编译
    $0 -j 8               # 使用 8 个并行任务编译

注意:
    此脚本已集成 H323PluginCodecManager 链接错误的修复方案

EOF
}

# 解析命令行参数
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -d|--debug-only)
                BUILD_DEBUG=yes
                BUILD_RELEASE=no
                shift
                ;;
            -r|--release-only)
                BUILD_DEBUG=no
                BUILD_RELEASE=yes
                shift
                ;;
            -j|--jobs)
                JOBS="$2"
                shift 2
                ;;
            -c|--clean)
                CLEAN_BUILD=yes
                shift
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            *)
                print_error "未知选项: $1"
                show_usage
                exit 1
                ;;
        esac
    done
}

# 检查源码目录是否存在
check_source_dirs() {
    print_step "检查源码目录..."
    
    local all_exist=true
    
    if [ ! -d "${PTLIB_SRC}" ]; then
        print_error "ptlib 源码目录不存在: ${PTLIB_SRC}"
        all_exist=false
    else
        print_info "找到 ptlib: ${PTLIB_SRC}"
    fi
    
    if [ ! -d "${H323PLUS_SRC}" ]; then
        print_error "h323plus 源码目录不存在: ${H323PLUS_SRC}"
        all_exist=false
    else
        print_info "找到 h323plus: ${H323PLUS_SRC}"
    fi
    
    if [ ! -d "${GNUGK_SRC}" ]; then
        print_error "gnugk 源码目录不存在: ${GNUGK_SRC}"
        all_exist=false
    else
        print_info "找到 gnugk: ${GNUGK_SRC}"
    fi
    
    if [ "$all_exist" = false ]; then
        print_error "请确保所有源码都在 ${THIRDPARTY_DIR} 目录下"
        exit 1
    fi
    
    print_success "✓ 源码目录检查通过"
}

# 创建目标目录
create_target_dirs() {
    print_step "创建目标目录..."
    
    mkdir -p "${INCLUDE_DIR}"
    mkdir -p "${DEBUG_LIB_DIR}"
    mkdir -p "${RELEASE_LIB_DIR}"
    
    if [ "${CLEAN_BUILD}" = "yes" ]; then
        print_warning "清理旧的编译目录..."
        rm -rf "${BUILD_DIR}"
    fi
    
    mkdir -p "${BUILD_DIR}"
    
    print_success "✓ 目标目录创建完成"
}

# 检查并安装依赖
check_dependencies() {
    print_step "检查编译依赖..."
    
    local missing_deps=()
    
    # 检查必需的编译工具
    command -v g++ >/dev/null 2>&1 || missing_deps+=("g++")
    command -v make >/dev/null 2>&1 || missing_deps+=("make")
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_error "缺少以下依赖: ${missing_deps[*]}"
        print_info "请安装: sudo apt-get install build-essential libssl-dev libexpat1-dev"
        exit 1
    fi
    
    # 检查可选但推荐的库
    local optional_libs=("libssl-dev" "libexpat1-dev")
    local missing_optional=()
    
    for lib in "${optional_libs[@]}"; do
        if ! dpkg -l | grep -q "^ii  $lib"; then
            missing_optional+=("$lib")
        fi
    done
    
    if [ ${#missing_optional[@]} -gt 0 ]; then
        print_warning "建议安装以下库: ${missing_optional[*]}"
        print_info "运行: sudo apt-get install ${missing_optional[*]}"
    fi
    
    print_success "✓ 编译工具检查通过"
}

################################################################################
# 编译 PTLib
################################################################################
build_ptlib() {
    local build_type=$1  # debug 或 release
    
    print_step "开始编译 PTLib (${build_type})..."
    
    cd "${PTLIB_SRC}"
    
    # 清理之前的编译
    if [ "${CLEAN_BUILD}" = "yes" ]; then
        print_info "清理 PTLib..."
        make clean 2>/dev/null || true
    fi
    
    # 设置编译标志 - 关键：添加 -fPIC 支持
    if [ "${build_type}" = "debug" ]; then
        export CFLAGS="-g -O0 -fPIC"
        export CXXFLAGS="-g -O0 -fPIC"
        export DEBUG=1
        PTLIB_CONFIG_OPTS="--enable-debug --enable-plugins"
    else
        export CFLAGS="-O2 -fPIC"
        export CXXFLAGS="-O2 -fPIC"
        unset DEBUG
        PTLIB_CONFIG_OPTS="--enable-plugins"
    fi
    
    # 配置
    if [ ! -f "Makefile" ] || [ "${CLEAN_BUILD}" = "yes" ]; then
        print_info "配置 PTLib..."
        if [ -f "configure" ]; then
            ./configure --prefix="${BUILD_DIR}/ptlib_${build_type}" ${PTLIB_CONFIG_OPTS} || \
                print_warning "configure 失败，尝试继续编译..."
        elif [ -f "configure.ac" ] || [ -f "configure.in" ]; then
            print_info "生成 configure 脚本..."
            autoconf 2>/dev/null || true
            if [ -f "configure" ]; then
                ./configure --prefix="${BUILD_DIR}/ptlib_${build_type}" ${PTLIB_CONFIG_OPTS} || \
                    print_warning "configure 失败，尝试继续编译..."
            fi
        else
            print_warning "未找到 configure 脚本，直接编译"
        fi
    fi
    
    # 编译
    print_info "编译 PTLib (使用 ${JOBS} 个并行任务)..."
    if ! make opt -j${JOBS} 2>&1 | tee /tmp/ptlib_build_${build_type}.log; then
        print_warning "并行编译失败，尝试串行编译..."
        make opt
    fi
    
    # 复制库文件
    local TARGET_LIB
    if [ "${build_type}" = "debug" ]; then
        TARGET_LIB="${DEBUG_LIB_DIR}"
    else
        TARGET_LIB="${RELEASE_LIB_DIR}"
    fi
    
    print_info "复制 PTLib 库文件到 ${TARGET_LIB}..."
    find . -name "*.so*" -type f -exec cp -P {} "${TARGET_LIB}/" \; 2>/dev/null || true
    find . -name "*.a" -type f -exec cp {} "${TARGET_LIB}/" \; 2>/dev/null || true
    find lib -name "*.so*" -type f -exec cp -P {} "${TARGET_LIB}/" \; 2>/dev/null || true
    find lib -name "*.a" -type f -exec cp {} "${TARGET_LIB}/" \; 2>/dev/null || true
    
    # 复制头文件 (只需要复制一次)
    if [ "${build_type}" = "release" ] || [ "${BUILD_RELEASE}" != "yes" ]; then
        print_info "复制 PTLib 头文件到 ${INCLUDE_DIR}..."
        if [ -d "include/ptlib" ]; then
            cp -r include/ptlib "${INCLUDE_DIR}/" 2>/dev/null || \
                print_warning "复制头文件失败"
        elif [ -d "include" ]; then
            mkdir -p "${INCLUDE_DIR}/ptlib"
            cp -r include/* "${INCLUDE_DIR}/ptlib/" 2>/dev/null || \
                print_warning "复制头文件失败"
        fi
    fi
    
    print_success "✓ PTLib (${build_type}) 编译完成"
}

################################################################################
# 编译 H323Plus - 关键修复：启用插件支持
################################################################################
build_h323plus() {
    local build_type=$1  # debug 或 release
    
    print_step "开始编译 H323Plus (${build_type}) - 启用插件支持..."
    
    cd "${H323PLUS_SRC}"
    
    # 设置 PTLIB 路径
    export PTLIBDIR="${PTLIB_SRC}"
    
    # 清理之前的编译
    if [ "${CLEAN_BUILD}" = "yes" ]; then
        print_info "清理 H323Plus..."
        make clean 2>/dev/null || true
    fi
    
    # 设置编译标志 - 关键修复：启用插件支持和 -fPIC
    if [ "${build_type}" = "debug" ]; then
        export CFLAGS="-g -O0 -fPIC"
        export CXXFLAGS="-g -O0 -fPIC"
        export DEBUG=1
    else
        export CFLAGS="-O2 -fPIC"
        export CXXFLAGS="-O2 -fPIC"
        unset DEBUG
    fi
    
    # *** 核心修复：启用插件编解码器支持 ***
    export H323_PLUGIN_CODECS=1
    print_info "✓ 已启用插件支持 (H323_PLUGIN_CODECS=1)"
    
    # 配置（如果有 configure）
    if [ -f "configure" ] && ([ ! -f "Makefile" ] || [ "${CLEAN_BUILD}" = "yes" ]); then
        print_info "配置 H323Plus..."
        ./configure --enable-plugins || print_warning "configure 失败，尝试继续编译..."
    fi
    
    # 编译
    print_info "编译 H323Plus (使用 ${JOBS} 个并行任务)..."
    if ! make opt -j${JOBS} 2>&1 | tee /tmp/h323plus_build_${build_type}.log; then
        print_warning "并行编译失败，尝试串行编译..."
        make opt
    fi
    
    # 验证插件支持
    print_info "验证 H323PluginCodecManager 符号..."
    if find lib -name "*.a" -exec nm {} \; 2>/dev/null | grep -q "H323PluginCodecManager"; then
        print_success "✓ 找到 H323PluginCodecManager 符号"
    else
        print_warning "⚠ 未找到 H323PluginCodecManager 符号，GnuGk 编译可能失败"
    fi
    
    # 复制库文件
    local TARGET_LIB
    if [ "${build_type}" = "debug" ]; then
        TARGET_LIB="${DEBUG_LIB_DIR}"
    else
        TARGET_LIB="${RELEASE_LIB_DIR}"
    fi
    
    print_info "复制 H323Plus 库文件到 ${TARGET_LIB}..."
    find . -name "*.so*" -type f -exec cp -P {} "${TARGET_LIB}/" \; 2>/dev/null || true
    find . -name "*.a" -type f -exec cp {} "${TARGET_LIB}/" \; 2>/dev/null || true
    find lib -name "*.so*" -type f -exec cp -P {} "${TARGET_LIB}/" \; 2>/dev/null || true
    find lib -name "*.a" -type f -exec cp {} "${TARGET_LIB}/" \; 2>/dev/null || true
    
    # 复制头文件 (只需要复制一次)
    if [ "${build_type}" = "release" ] || [ "${BUILD_RELEASE}" != "yes" ]; then
        print_info "复制 H323Plus 头文件到 ${INCLUDE_DIR}..."
        if [ -d "include" ]; then
            cp -r include/* "${INCLUDE_DIR}/" 2>/dev/null || \
                print_warning "复制头文件失败"
        fi
    fi
    
    print_success "✓ H323Plus (${build_type}) 编译完成"
}

################################################################################
# 编译 GnuGk - 集成链接修复方案
################################################################################
build_gnugk() {
    local build_type=$1  # debug 或 release
    
    print_step "开始编译 GnuGk (${build_type})..."
    
    cd "${GNUGK_SRC}"
    
    # 设置 PTLIB 和 H323PLUS 路径
    export PTLIBDIR="${PTLIB_SRC}"
    export OPENH323DIR="${H323PLUS_SRC}"
    export H323PLUSDIR="${H323PLUS_SRC}"  # 某些版本需要
    
    # 清理之前的编译
    if [ "${CLEAN_BUILD}" = "yes" ]; then
        print_info "清理 GnuGk..."
        make clean 2>/dev/null || true
    fi
    
    # 设置编译标志
    if [ "${build_type}" = "debug" ]; then
        export CFLAGS="-g -O0 -fPIC"
        export CXXFLAGS="-g -O0 -fPIC"
        export DEBUG=1
    else
        export CFLAGS="-O2 -fPIC"
        export CXXFLAGS="-O2 -fPIC"
        unset DEBUG
    fi
    
    # 配置
    if [ ! -f "Makefile" ] || [ "${CLEAN_BUILD}" = "yes" ]; then
        print_info "配置 GnuGk..."
        if [ -f "configure" ]; then
            ./configure \
                --prefix="${BUILD_DIR}/gnugk_${build_type}" \
                --with-ptlibdir="${PTLIBDIR}" \
                --with-openh323dir="${OPENH323DIR}" \
                || print_warning "configure 失败，尝试继续编译..."
        elif [ -f "configure.ac" ]; then
            autoconf 2>/dev/null || true
            if [ -f "configure" ]; then
                ./configure \
                    --prefix="${BUILD_DIR}/gnugk_${build_type}" \
                    --with-ptlibdir="${PTLIBDIR}" \
                    --with-openh323dir="${OPENH323DIR}" \
                    || print_warning "configure 失败，尝试继续编译..."
            fi
        fi
    fi
    
    # 编译 - 使用单线程避免链接顺序问题
    print_info "编译 GnuGk (单线程以确保链接正确)..."
    
    # 方法 1: 尝试正常编译
    if make opt 2>&1 | tee /tmp/gnugk_build_${build_type}.log; then
        print_success "✓ GnuGk 常规编译成功"
    else
        print_warning "常规编译失败，尝试手动链接修复..."
        
        # 方法 2: 手动链接 - 使用 start-group/end-group 解决循环依赖
        print_info "收集目标文件和库文件..."
        
        OBJ_FILES=$(find . -name "*.o" -path "*/obj_*" 2>/dev/null)
        H323_LIB=$(find "${OPENH323DIR}" -name "libh323*.a" 2>/dev/null | head -1)
        PT_LIB=$(find "${PTLIBDIR}" -name "libpt*.a" 2>/dev/null | head -1)
        
        if [ -z "$OBJ_FILES" ]; then
            print_error "未找到目标文件 (.o)，编译可能未完成"
            print_info "查看日志: /tmp/gnugk_build_${build_type}.log"
            return 1
        fi
        
        if [ -z "$H323_LIB" ] || [ -z "$PT_LIB" ]; then
            print_error "未找到必要的库文件"
            print_info "H323 库: ${H323_LIB:-未找到}"
            print_info "PT 库: ${PT_LIB:-未找到}"
            return 1
        fi
        
        print_info "找到的库文件:"
        print_info "  H323Plus: $H323_LIB"
        print_info "  PTLib: $PT_LIB"
        
        print_info "手动链接 GnuGk (使用 --start-group/--end-group)..."
        
        # 使用 start-group 和 end-group 解决循环依赖
        if g++ -o gnugk \
            -Wl,--start-group \
            $OBJ_FILES \
            "$H323_LIB" \
            "$PT_LIB" \
            -Wl,--end-group \
            -lpthread -ldl -lrt -lssl -lcrypto -lexpat -lm -lstdc++ \
            2>&1 | tee /tmp/gnugk_link_${build_type}.log; then
            print_success "✓ 手动链接成功！"
        else
            print_error "手动链接失败"
            print_info "查看链接日志: /tmp/gnugk_link_${build_type}.log"
            return 1
        fi
    fi
    
    # 复制文件
    local TARGET_LIB
    if [ "${build_type}" = "debug" ]; then
        TARGET_LIB="${DEBUG_LIB_DIR}"
    else
        TARGET_LIB="${RELEASE_LIB_DIR}"
    fi
    
    print_info "复制 GnuGk 文件到 ${TARGET_LIB}..."
    
    # 复制可执行文件
    if [ -f "gnugk" ]; then
        cp gnugk "${TARGET_LIB}/"
        chmod +x "${TARGET_LIB}/gnugk"
        print_success "✓ 复制 gnugk 可执行文件"
    else
        find . -name "gnugk" -type f -executable -exec cp {} "${TARGET_LIB}/" \; 2>/dev/null || \
            print_warning "未找到 gnugk 可执行文件"
    fi
    
    # 复制库文件（如果有）
    find . -name "*.so*" -type f -exec cp -P {} "${TARGET_LIB}/" \; 2>/dev/null || true
    find . -name "*.a" -type f -exec cp {} "${TARGET_LIB}/" \; 2>/dev/null || true
    
    # 复制头文件（如果有）
    if [ "${build_type}" = "release" ] || [ "${BUILD_RELEASE}" != "yes" ]; then
        if [ -d "include" ]; then
            print_info "复制 GnuGk 头文件到 ${INCLUDE_DIR}..."
            cp -r include/* "${INCLUDE_DIR}/" 2>/dev/null || true
        fi
    fi
    
    # 验证 gnugk
    if [ -f "${TARGET_LIB}/gnugk" ]; then
        print_info "验证 GnuGk 可执行文件..."
        if "${TARGET_LIB}/gnugk" --version 2>/dev/null | head -1; then
            print_success "✓ GnuGk 可以正常运行"
        else
            print_warning "GnuGk 编译完成，但运行时可能需要设置 LD_LIBRARY_PATH"
        fi
        
        print_info "检查依赖库..."
        if ldd "${TARGET_LIB}/gnugk" 2>/dev/null | grep -q "not found"; then
            print_warning "发现缺失的依赖库:"
            ldd "${TARGET_LIB}/gnugk" | grep "not found"
        else
            print_success "✓ 所有依赖库都已找到"
        fi
    fi
    
    print_success "✓ GnuGk (${build_type}) 编译完成"
}

################################################################################
# 显示编译结果摘要
################################################################################
show_summary() {
    echo ""
    echo "════════════════════════════════════════════════════════════════"
    print_success "✓ 所有编译任务完成！"
    echo "════════════════════════════════════════════════════════════════"
    echo ""
    
    print_info "安装位置:"
    print_info "  头文件: ${INCLUDE_DIR}"
    
    if [ "${BUILD_DEBUG}" = "yes" ]; then
        print_info "  Debug 库: ${DEBUG_LIB_DIR}"
    fi
    
    if [ "${BUILD_RELEASE}" = "yes" ]; then
        print_info "  Release 库: ${RELEASE_LIB_DIR}"
    fi
    
    echo ""
    
    # 显示库文件列表
    if [ "${BUILD_DEBUG}" = "yes" ] && [ -d "${DEBUG_LIB_DIR}" ]; then
        print_info "Debug 库文件:"
        if ls -lh "${DEBUG_LIB_DIR}"/*.{so,a} 2>/dev/null | head -20; then
            local count=$(ls "${DEBUG_LIB_DIR}"/*.{so,a} 2>/dev/null | wc -l)
            [ $count -gt 20 ] && print_info "  ... (共 $count 个文件)"
        else
            print_warning "  目录为空或无库文件"
        fi
        echo ""
    fi
    
    if [ "${BUILD_RELEASE}" = "yes" ] && [ -d "${RELEASE_LIB_DIR}" ]; then
        print_info "Release 库文件:"
        if ls -lh "${RELEASE_LIB_DIR}"/*.{so,a} 2>/dev/null | head -20; then
            local count=$(ls "${RELEASE_LIB_DIR}"/*.{so,a} 2>/dev/null | wc -l)
            [ $count -gt 20 ] && print_info "  ... (共 $count 个文件)"
        else
            print_warning "  目录为空或无库文件"
        fi
        echo ""
    fi
    
    # 显示如何使用
    echo "════════════════════════════════════════════════════════════════"
    print_info "使用方法:"
    echo "════════════════════════════════════════════════════════════════"
    echo ""
    echo "在你的项目中使用这些库:"
    echo ""
    echo "# CMake 示例"
    echo "set(H323_INCLUDE_DIR \"\${CMAKE_SOURCE_DIR}/../10-common/include\")"
    echo "set(H323_LIB_DIR \"\${CMAKE_SOURCE_DIR}/../10-common/lib/releaselib/linux64/release\")"
    echo ""
    echo "include_directories(\${H323_INCLUDE_DIR})"
    echo "target_link_libraries(your_target"
    echo "    \${H323_LIB_DIR}/libh323.a"
    echo "    \${H323_LIB_DIR}/libpt.a"
    echo "    pthread dl rt ssl crypto expat"
    echo ")"
    echo ""
    echo "# 直接使用 g++"
    echo "g++ -I../10-common/include your_code.cpp \\"
    echo "    -L../10-common/lib/releaselib/linux64/release \\"
    echo "    -lh323 -lpt -lpthread -ldl -lrt -lssl -lcrypto -lexpat \\"
    echo "    -o your_program"
    echo ""
}

################################################################################
# 主编译流程
################################################################################
main() {
    echo "════════════════════════════════════════════════════════════════"
    echo "  H.323 库完整编译脚本"
    echo "  编译: PTLib, H323Plus, GnuGk"
    echo "  集成修复: H323PluginCodecManager 链接错误"
    echo "════════════════════════════════════════════════════════════════"
    echo ""
    
    # 解析参数
    parse_args "$@"
    
    # 显示配置
    print_info "项目根目录: ${PROJECT_ROOT}"
    print_info "源码目录: ${THIRDPARTY_DIR}"
    print_info "安装目录: ${COMMON_DIR}"
    print_info "并行任务数: ${JOBS}"
    print_info "编译 Debug: ${BUILD_DEBUG}"
    print_info "编译 Release: ${BUILD_RELEASE}"
    echo ""
    
    # 检查环境
    check_source_dirs
    check_dependencies
    create_target_dirs
    
    # 保存当前目录
    ORIGINAL_DIR=$(pwd)
    
    # 编译 Debug 版本
    if [ "${BUILD_DEBUG}" = "yes" ]; then
        echo ""
        echo "════════════════════════════════════════════════════════════════"
        echo "  编译 Debug 版本"
        echo "════════════════════════════════════════════════════════════════"
        
        build_ptlib "debug"
        build_h323plus "debug"
        build_gnugk "debug"
    fi
    
    # 编译 Release 版本
    if [ "${BUILD_RELEASE}" = "yes" ]; then
        echo ""
        echo "════════════════════════════════════════════════════════════════"
        echo "  编译 Release 版本"
        echo "════════════════════════════════════════════════════════════════"
        
        build_ptlib "release"
        build_h323plus "release"
        build_gnugk "release"
    fi
    
    # 返回原目录
    cd "${ORIGINAL_DIR}"
    
    # 显示摘要
    show_summary
}

# 捕获错误
trap 'print_error "编译过程中发生错误，退出码: $?"; exit 1' ERR

# 执行主函数
main "$@"
