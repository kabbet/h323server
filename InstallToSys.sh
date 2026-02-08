#!/bin/bash
set -e  # 遇到错误立即退出

# ============================================================
# 配置变量
# ============================================================
PG_VERSION="17.2"              # PostgreSQL 版本
REDIS_VERSION="7.2.4"          # Redis 版本
HIREDIS_VERSION="1.2.0"        # hiredis 版本

BASE_INSTALL_DIR="/opt/mcu"
PG_INSTALL_DIR="${BASE_INSTALL_DIR}/postgresql"
REDIS_INSTALL_DIR="${BASE_INSTALL_DIR}/redis"
HIREDIS_INSTALL_DIR="${BASE_INSTALL_DIR}/hiredis"

PG_DOWNLOAD_URL="https://ftp.postgresql.org/pub/source/v${PG_VERSION}/postgresql-${PG_VERSION}.tar.gz"
REDIS_DOWNLOAD_URL="https://download.redis.io/releases/redis-${REDIS_VERSION}.tar.gz"
HIREDIS_DOWNLOAD_URL="https://github.com/redis/hiredis/archive/v${HIREDIS_VERSION}.tar.gz"

BUILD_DIR="/tmp/mcu-build"

# ============================================================
# 颜色输出
# ============================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# ============================================================
# 检查权限
# ============================================================
check_privileges() {
    if [ "$EUID" -ne 0 ]; then
        echo_warn "脚本需要 root 权限来创建目录和安装依赖包"
        if ! command -v sudo &> /dev/null; then
            echo_error "sudo 命令不存在，请以 root 用户运行此脚本"
            exit 1
        fi
        SUDO="sudo"
    else
        SUDO=""
    fi
}

# ============================================================
# 检测操作系统
# ============================================================
detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$ID
        OS_VERSION=$VERSION_ID
    else
        echo_error "无法检测操作系统类型"
        exit 1
    fi
    echo_info "检测到操作系统: $OS $OS_VERSION"
}

# ============================================================
# 安装编译依赖
# ============================================================
install_dependencies() {
    echo_step "安装编译依赖包..."
    
    case $OS in
        ubuntu|debian)
            $SUDO apt-get update
            $SUDO apt-get install -y \
                build-essential \
                libreadline-dev \
                zlib1g-dev \
                flex \
                bison \
                libxml2-dev \
                libxslt-dev \
                libssl-dev \
                libicu-dev \
                pkg-config \
                wget \
                cmake \
                tcl \
                tcl-dev
            ;;
        centos|rhel|rocky|almalinux)
            $SUDO yum groupinstall -y "Development Tools"
            $SUDO yum install -y \
                readline-devel \
                zlib-devel \
                flex \
                bison \
                libxml2-devel \
                libxslt-devel \
                openssl-devel \
                libicu-devel \
                pkg-config \
                wget \
                cmake \
                tcl \
                tcl-devel
            ;;
        fedora)
            $SUDO dnf groupinstall -y "Development Tools"
            $SUDO dnf install -y \
                readline-devel \
                zlib-devel \
                flex \
                bison \
                libxml2-devel \
                libxslt-devel \
                openssl-devel \
                libicu-devel \
                pkg-config \
                wget \
                cmake \
                tcl \
                tcl-devel
            ;;
        *)
            echo_error "不支持的操作系统: $OS"
            echo_warn "请手动安装编译依赖"
            read -p "是否继续? (y/n) " -n 1 -r
            echo
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                exit 1
            fi
            ;;
    esac
    
    echo_info "依赖包安装完成"
}

# ============================================================
# 创建安装目录
# ============================================================
create_install_dirs() {
    echo_step "创建安装目录..."
    $SUDO mkdir -p "$BASE_INSTALL_DIR"
    $SUDO mkdir -p "$PG_INSTALL_DIR"
    $SUDO mkdir -p "$REDIS_INSTALL_DIR"
    $SUDO mkdir -p "$HIREDIS_INSTALL_DIR"
    
    # 如果不是 root，将目录所有权更改为当前用户
    if [ -n "$SUDO" ]; then
        $SUDO chown -R $(whoami):$(id -gn) "$BASE_INSTALL_DIR"
    fi
    
    echo_info "目录创建完成"
}

# ============================================================
# 编译安装 hiredis
# ============================================================
install_hiredis() {
    echo_step "=========================================="
    echo_step "开始编译安装 hiredis ${HIREDIS_VERSION}"
    echo_step "=========================================="
    
    cd "$BUILD_DIR"
    
    # 下载
    echo_info "下载 hiredis ${HIREDIS_VERSION} 源码..."
    if [ -f "hiredis-${HIREDIS_VERSION}.tar.gz" ]; then
        echo_warn "源码包已存在，跳过下载"
    else
        wget "$HIREDIS_DOWNLOAD_URL" -O "hiredis-${HIREDIS_VERSION}.tar.gz"
    fi
    
    # 解压
    echo_info "解压源码包..."
    tar -xzf "hiredis-${HIREDIS_VERSION}.tar.gz"
    cd "hiredis-${HIREDIS_VERSION}"
    
    # 编译安装
    echo_info "编译 hiredis..."
    make -j$(nproc)
    
    echo_info "安装 hiredis 到 ${HIREDIS_INSTALL_DIR}..."
    make PREFIX="${HIREDIS_INSTALL_DIR}" install
    
    echo_info "✓ hiredis 安装完成"
}

# ============================================================
# 编译安装 Redis
# ============================================================
install_redis() {
    echo_step "=========================================="
    echo_step "开始编译安装 Redis ${REDIS_VERSION}"
    echo_step "=========================================="
    
    cd "$BUILD_DIR"
    
    # 下载
    echo_info "下载 Redis ${REDIS_VERSION} 源码..."
    if [ -f "redis-${REDIS_VERSION}.tar.gz" ]; then
        echo_warn "源码包已存在，跳过下载"
    else
        wget "$REDIS_DOWNLOAD_URL" -O "redis-${REDIS_VERSION}.tar.gz"
    fi
    
    # 解压
    echo_info "解压源码包..."
    tar -xzf "redis-${REDIS_VERSION}.tar.gz"
    cd "redis-${REDIS_VERSION}"
    
    # 编译
    echo_info "编译 Redis (这可能需要几分钟)..."
    make -j$(nproc) USE_SYSTEMD=no BUILD_TLS=yes
    
    # 测试（可选，注释掉以加快安装速度）
    # echo_info "运行 Redis 测试..."
    # make test
    
    # 安装
    echo_info "安装 Redis 到 ${REDIS_INSTALL_DIR}..."
    make PREFIX="${REDIS_INSTALL_DIR}" install
    
    # 复制配置文件
    echo_info "复制配置文件..."
    mkdir -p "${REDIS_INSTALL_DIR}/etc"
    cp redis.conf "${REDIS_INSTALL_DIR}/etc/"
    cp sentinel.conf "${REDIS_INSTALL_DIR}/etc/" 2>/dev/null || true
    
    # 创建必要的目录
    mkdir -p "${REDIS_INSTALL_DIR}/data"
    mkdir -p "${REDIS_INSTALL_DIR}/logs"
    
    # 修改配置文件中的路径
    sed -i "s|^dir .*|dir ${REDIS_INSTALL_DIR}/data|g" "${REDIS_INSTALL_DIR}/etc/redis.conf"
    sed -i "s|^logfile .*|logfile ${REDIS_INSTALL_DIR}/logs/redis.log|g" "${REDIS_INSTALL_DIR}/etc/redis.conf"
    sed -i "s|^pidfile .*|pidfile ${REDIS_INSTALL_DIR}/redis.pid|g" "${REDIS_INSTALL_DIR}/etc/redis.conf"
    
    # 设置为后台运行
    sed -i "s|^daemonize no|daemonize yes|g" "${REDIS_INSTALL_DIR}/etc/redis.conf"
    
    echo_info "✓ Redis 安装完成"
}

# ============================================================
# 编译安装 PostgreSQL
# ============================================================
install_postgresql() {
    echo_step "=========================================="
    echo_step "开始编译安装 PostgreSQL ${PG_VERSION}"
    echo_step "=========================================="
    
    cd "$BUILD_DIR"
    
    # 下载
    echo_info "下载 PostgreSQL ${PG_VERSION} 源码..."
    if [ -f "postgresql-${PG_VERSION}.tar.gz" ]; then
        echo_warn "源码包已存在，跳过下载"
    else
        wget "$PG_DOWNLOAD_URL" -O "postgresql-${PG_VERSION}.tar.gz"
    fi
    
    # 解压
    echo_info "解压源码包..."
    tar -xzf "postgresql-${PG_VERSION}.tar.gz"
    cd "postgresql-${PG_VERSION}"
    
    # 配置
    echo_info "运行 configure..."
    ./configure \
        --prefix="$PG_INSTALL_DIR" \
        --with-openssl \
        --with-libxml \
        --with-libxslt \
        --with-icu
    
    # 编译
    echo_info "编译 PostgreSQL (这可能需要几分钟)..."
    make -j$(nproc)
    
    # 安装
    echo_info "安装 PostgreSQL..."
    make install
    
    # 编译并安装 contrib 模块
    echo_info "编译并安装 contrib 模块..."
    cd contrib
    make -j$(nproc)
    make install
    
    echo_info "✓ PostgreSQL 安装完成"
}

# ============================================================
# 配置环境变量
# ============================================================
configure_environment() {
    echo_step "配置环境变量..."
    
    # 检测当前使用的 shell
    CURRENT_SHELL=$(basename "$SHELL")
    
    if [ "$CURRENT_SHELL" = "zsh" ]; then
        RC_FILE="$HOME/.zshrc"
    else
        RC_FILE="$HOME/.bashrc"
    fi
    
    echo_info "检测到 shell: $CURRENT_SHELL, 配置文件: $RC_FILE"
    
    # 创建环境变量配置
    ENV_CONFIG="
# ============================================================
# MCU Software Environment Variables
# ============================================================

# PostgreSQL
export PGHOME=\"$PG_INSTALL_DIR\"
export PGDATA=\"\$PGHOME/data\"
export PGHOST=localhost
export PGPORT=5432
export PGUSER=postgres

# Redis
export REDIS_HOME=\"$REDIS_INSTALL_DIR\"

# hiredis
export HIREDIS_HOME=\"$HIREDIS_INSTALL_DIR\"

# PATH
export PATH=\"\$PGHOME/bin:\$REDIS_HOME/bin:\$PATH\"

# Library Path
export LD_LIBRARY_PATH=\"\$PGHOME/lib:\$HIREDIS_HOME/lib:\$LD_LIBRARY_PATH\"

# Man Path
export MANPATH=\"\$PGHOME/share/man:\$MANPATH\"

# PKG_CONFIG_PATH for hiredis
export PKG_CONFIG_PATH=\"\$HIREDIS_HOME/lib/pkgconfig:\$PKG_CONFIG_PATH\"
"
    
    # 检查是否已经配置过
    if grep -q "# MCU Software Environment Variables" "$RC_FILE" 2>/dev/null; then
        echo_warn "环境变量已存在于 $RC_FILE，跳过配置"
    else
        echo "$ENV_CONFIG" >> "$RC_FILE"
        echo_info "环境变量已添加到 $RC_FILE"
    fi
    
    # 同时也添加到 .bash_profile (如果存在且是 bash)
    if [ -f "$HOME/.bash_profile" ] && [ "$CURRENT_SHELL" = "bash" ]; then
        if ! grep -q "# MCU Software Environment Variables" "$HOME/.bash_profile" 2>/dev/null; then
            echo "$ENV_CONFIG" >> "$HOME/.bash_profile"
            echo_info "环境变量已添加到 .bash_profile"
        fi
    fi
    
    # 立即加载环境变量（用于后续脚本）
    export PGHOME="$PG_INSTALL_DIR"
    export REDIS_HOME="$REDIS_INSTALL_DIR"
    export HIREDIS_HOME="$HIREDIS_INSTALL_DIR"
    export PATH="$PGHOME/bin:$REDIS_HOME/bin:$PATH"
    export LD_LIBRARY_PATH="$PGHOME/lib:$HIREDIS_HOME/lib:$LD_LIBRARY_PATH"
}

# ============================================================
# 清理构建目录
# ============================================================
cleanup() {
    echo_step "清理构建目录..."
    rm -rf "$BUILD_DIR"
    echo_info "清理完成"
}

# ============================================================
# 显示安装摘要
# ============================================================
show_summary() {
    echo
    echo_step "============================================================"
    echo_step "安装完成！"
    echo_step "============================================================"
    echo
    echo "已安装的软件:"
    echo "  1. hiredis ${HIREDIS_VERSION}  -> ${HIREDIS_INSTALL_DIR}"
    echo "  2. Redis ${REDIS_VERSION}      -> ${REDIS_INSTALL_DIR}"
    echo "  3. PostgreSQL ${PG_VERSION}    -> ${PG_INSTALL_DIR}"
    echo
    echo "请执行以下命令使环境变量生效:"
    if [ "$CURRENT_SHELL" = "zsh" ]; then
        echo "  source ~/.zshrc"
    else
        echo "  source ~/.bashrc"
    fi
    echo
    echo_step "============================================================"
    echo_step "Redis 使用说明"
    echo_step "============================================================"
    echo
    echo "1. 启动 Redis:"
    echo "   ${REDIS_INSTALL_DIR}/bin/redis-server ${REDIS_INSTALL_DIR}/etc/redis.conf"
    echo
    echo "2. 连接 Redis:"
    echo "   ${REDIS_INSTALL_DIR}/bin/redis-cli"
    echo
    echo "3. 停止 Redis:"
    echo "   ${REDIS_INSTALL_DIR}/bin/redis-cli shutdown"
    echo
    echo "4. 查看 Redis 状态:"
    echo "   ${REDIS_INSTALL_DIR}/bin/redis-cli ping"
    echo
    echo "配置文件位置: ${REDIS_INSTALL_DIR}/etc/redis.conf"
    echo "数据目录: ${REDIS_INSTALL_DIR}/data"
    echo "日志目录: ${REDIS_INSTALL_DIR}/logs"
    echo
    echo_step "============================================================"
    echo_step "PostgreSQL 使用说明"
    echo_step "============================================================"
    echo
    echo "1. 初始化数据库目录:"
    echo "   ${PG_INSTALL_DIR}/bin/initdb -D ${PG_INSTALL_DIR}/data -U postgres"
    echo
    echo "2. 启动 PostgreSQL:"
    echo "   ${PG_INSTALL_DIR}/bin/pg_ctl -D ${PG_INSTALL_DIR}/data -l ${PG_INSTALL_DIR}/logfile start"
    echo
    echo "3. 创建数据库:"
    echo "   ${PG_INSTALL_DIR}/bin/createdb -U postgres mydb"
    echo
    echo "4. 连接数据库:"
    echo "   ${PG_INSTALL_DIR}/bin/psql -U postgres mydb"
    echo
    echo "5. 停止 PostgreSQL:"
    echo "   ${PG_INSTALL_DIR}/bin/pg_ctl -D ${PG_INSTALL_DIR}/data stop"
    echo
    echo_step "============================================================"
    echo_step "hiredis 使用说明"
    echo_step "============================================================"
    echo
    echo "hiredis 是 C 语言的 Redis 客户端库，已安装到:"
    echo "  ${HIREDIS_INSTALL_DIR}"
    echo
    echo "在你的 C/C++ 项目中使用 hiredis:"
    echo
    echo "编译时添加参数:"
    echo "  gcc -I${HIREDIS_INSTALL_DIR}/include -L${HIREDIS_INSTALL_DIR}/lib -lhiredis your_program.c -o your_program"
    echo
    echo "或使用 pkg-config:"
    echo "  gcc \$(pkg-config --cflags --libs hiredis) your_program.c -o your_program"
    echo
    echo_step "============================================================"
    echo
}

# ============================================================
# 主函数
# ============================================================
main() {
    echo_step "============================================================"
    echo_step "MCU 软件源码编译安装脚本"
    echo_step "============================================================"
    echo
    echo "将要安装以下软件:"
    echo "  - hiredis ${HIREDIS_VERSION}"
    echo "  - Redis ${REDIS_VERSION}"
    echo "  - PostgreSQL ${PG_VERSION}"
    echo
    echo "安装路径: ${BASE_INSTALL_DIR}"
    echo
    
    # 询问是否继续
    read -p "是否继续安装? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo_warn "安装已取消"
        exit 0
    fi
    
    # 检查权限和环境
    check_privileges
    detect_os
    
    # 安装依赖
    install_dependencies
    
    # 创建构建目录
    echo_step "创建构建目录: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    
    # 创建安装目录
    create_install_dirs
    
    # 按顺序安装（hiredis -> Redis -> PostgreSQL）
    install_hiredis
    install_redis
    install_postgresql
    
    # 配置环境
    configure_environment
    
    # 清理
    cleanup
    
    # 显示摘要
    show_summary
}

# 运行主函数
main
