#!/bin/bash

# Models 链接问题诊断脚本

echo "=========================================="
echo "Drogon ORM Models 链接问题诊断"
echo "=========================================="
echo ""

HTTPSERVER_ROOT="../"

echo "1. 检查 Models 头文件："
echo "--------------------------------------"
if [ -f "$HTTPSERVER_ROOT/include/models/Users.h" ]; then
    echo "✓ Users.h 存在"
else
    echo "✗ Users.h 不存在"
fi

if [ -f "$HTTPSERVER_ROOT/include/models/UserTokens.h" ]; then
    echo "✓ UserTokens.h 存在"
else
    echo "✗ UserTokens.h 不存在"
fi

echo ""
echo "2. 查找 Models 实现文件 (.cc 或 .cpp)："
echo "--------------------------------------"

MODELS_IMPL=$(find "$HTTPSERVER_ROOT" -name "Users.cc" -o -name "Users.cpp" -o -name "UserTokens.cc" -o -name "UserTokens.cpp" 2>/dev/null)

if [ -z "$MODELS_IMPL" ]; then
    echo "✗ 未找到 Models 实现文件！"
    echo ""
    echo "Drogon ORM 生成的 Models 通常在："
    echo "  - models/ 目录"
    echo "  - source/models/ 目录"
    echo ""
    
    echo "3. 检查可能的位置："
    echo "--------------------------------------"
    
    # 检查常见位置
    for dir in "models" "source/models" "source" "src/models"; do
        FULL_PATH="$HTTPSERVER_ROOT/$dir"
        if [ -d "$FULL_PATH" ]; then
            echo "检查: $FULL_PATH"
            ls -la "$FULL_PATH" | grep -E "\.(cc|cpp)$" || echo "  (无 .cc/.cpp 文件)"
        fi
    done
else
    echo "✓ 找到 Models 实现文件："
    echo "$MODELS_IMPL"
fi

echo ""
echo "4. 检查 drogon_ctl 命令："
echo "--------------------------------------"

DROGON_CTL="../../../10-common/version/bin/debug/drogon_ctl"

if [ -f "$DROGON_CTL" ]; then
    echo "✓ drogon_ctl 存在: $DROGON_CTL"
    echo ""
    echo "Drogon 版本信息："
    $DROGON_CTL version | grep "Version:"
else
    echo "✗ drogon_ctl 不存在"
fi

echo ""
echo "5. 检查数据库配置："
echo "--------------------------------------"

CONFIG_FILE="$HTTPSERVER_ROOT/config.json"
if [ -f "$CONFIG_FILE" ]; then
    echo "✓ 找到配置文件: $CONFIG_FILE"
    
    # 检查数据库配置
    if grep -q "\"rdbms\"" "$CONFIG_FILE"; then
        echo ""
        echo "数据库配置（部分）："
        grep -A 10 "\"rdbms\"" "$CONFIG_FILE" | head -15
    fi
else
    echo "✗ 未找到 config.json"
fi

echo ""
echo "=========================================="
echo "解决方案建议"
echo "=========================================="
echo ""

echo "问题：Drogon ORM Models 的实现文件 (.cc) 没有被编译"
echo ""
echo "方案 1: 使用 drogon_ctl 生成 Models（推荐）"
echo "--------------------------------------"
echo "cd $HTTPSERVER_ROOT"
echo "$DROGON_CTL create model myapp"
echo ""
echo "这将根据数据库生成完整的 Models 文件（.h 和 .cc）"
echo ""

echo "方案 2: 手动添加现有的 Models 实现文件到 CMakeLists.txt"
echo "--------------------------------------"
echo "编辑: tests/CMakeLists.txt"
echo ""
echo "在 PROJECT_SOURCES 中添加："
echo "set(PROJECT_SOURCES"
echo "    \${HTTPSERVER_ROOT}/source/services/UserService.cpp"
echo "    \${HTTPSERVER_ROOT}/models/Users.cc          # ← 添加"
echo "    \${HTTPSERVER_ROOT}/models/UserTokens.cc     # ← 添加"
echo ")"
echo ""

echo "方案 3: 链接主项目的对象文件"
echo "--------------------------------------"
echo "如果主项目已经编译了 Models，可以链接其对象文件或静态库"
echo ""

echo "=========================================="
