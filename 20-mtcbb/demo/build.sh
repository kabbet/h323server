#!/bin/bash
#!/bin/bash

set -e  # 遇到错误立即退出

# ============ 配置部分 ============
PROJECT_NAME=$(basename "$PWD")
COMPILE_OUTPUT_DIR="../../../10-common/version/compileinfo"
COMPILE_OUTPUT_BASE="${COMPILE_OUTPUT_DIR}/${PROJECT_NAME}_linux64_cmake"
COMPILE_INSTALL_DIR="../../../10-common/version/bin/"

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

# 编译函数
build_version() {
    local version_type=$1
    local build_path="${version_type}_build"
    local output_file="${COMPILE_OUTPUT_BASE}_${version_type}.txt"
    
    print_step "=========================================="
    print_step "开始编译 ${version_type} 版本"
    print_step "=========================================="
    
    # 清理旧的构建目录
    if [ -d "${build_path}" ]; then
        print_warning "清理旧的构建目录: ${build_path}"
        rm -rf "${build_path}"
    fi
    
    # 创建构建目录
    print_info "创建构建目录: ${build_path}"
    mkdir -p "${build_path}"
    cd "${build_path}"

    # 确保输出目录存在
    mkdir -p "${COMPILE_OUTPUT_DIR}"
    
    # CMake 配置
    print_info "CMake 配置中..."
    if [ "${version_type}" == "Debug" ];then
        cmake -G Ninja \
                -DCMAKE_VERBOSE_MAKEFILE=ON \
                -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
                -DCMAKE_INSTALL_PREFIX=${COMPILE_INSTALL_DIR}/$1 \
                -DCMAKE_BUILD_TYPE="${version_type}" \
                .. > /dev/null 2>&1  || {
                print_error "CMake 配置失败"
                cd ..
                return 1
            }

    else
        cmake -G Ninja \
                -DCMAKE_VERBOSE_MAKEFILE=OFF \
                -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
                -DCMAKE_INSTALL_PREFIX=${COMPILE_INSTALL_DIR}/$1 \
                -DCMAKE_BUILD_TYPE="${version_type}" \
                .. > /dev/null 2>&1 || {
                print_error "CMake 配置失败"
                cd ..
                return 1
            }
    fi
    
    # 编译
    print_info "开始编译 (使用 $(nproc) 个并行任务)..."
    
    # 确保输出目录存在
    mkdir -p "$(dirname "${output_file}")"
    
    if ninja -j"$(nproc)" -v > "${output_file}" 2>&1; then
        print_info "编译成功！编译日志: ${output_file}"
    else
        print_error "编译失败！查看日志: ${output_file}"
        cd ..
        return 1
    fi

    
    print_step "${version_type} install done"
    
    # 返回上级目录
    cd ..
    
    print_step "${version_type} 版本编译完成"
    rm -rf ${build_path}
    echo ""
}

# ============ 主流程 ============
main() {
    local start_time=$(date +%s)
    
    print_step "=========================================="
    print_step "项目: ${PROJECT_NAME}"
    print_step "编译输出: ${COMPILE_OUTPUT_DIR}"
    print_step "=========================================="
    echo ""
   
    # 编译 Debug 版本
    if ! build_version "Debug"; then
        print_error "Debug 版本编译失败"
        exit 1
    fi
    
    # 编译 Release 版本
    if ! build_version "Release"; then
        print_error "Release 版本编译失败"
        exit 1
    fi
    
    # 计算总耗时
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    print_step "=========================================="
    print_step "所有版本编译完成！"
    print_step "总耗时: ${duration} 秒"
    print_step "=========================================="
}

# 执行主流程
main
