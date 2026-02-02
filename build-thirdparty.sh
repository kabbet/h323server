#!/bin/bash

set -e

BUILD_TYPE=${1:-Debug}
PROJECT_ROOT=$(pwd)

# 检查系统是否安装了 libjsoncpp-dev
if dpkg -l | grep -q libjsoncpp-dev; then
    echo "========================================="
    echo "警告: 检测到系统安装了 libjsoncpp-dev"
    echo "这可能导致 drogon 使用系统版本而非自定义版本"
    echo "========================================="
    read -p "是否临时卸载系统的 libjsoncpp-dev? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo apt-get remove -y libjsoncpp-dev
        REINSTALL_JSONCPP=true
    else
        echo "继续编译，但可能出现版本冲突..."
    fi
fi

echo "Building third-party libraries in ${BUILD_TYPE} mode..."

git submodule init
git submodule update --init --recursive

# 根据构建类型设置目录
if [ "${BUILD_TYPE}" = "Debug" ]; then
    BUILD_TYPE_LOWER="debug"
else
    BUILD_TYPE_LOWER="release"
fi

# 安装路径（区分 debug/release）
THIRDPARTY_INCLUDE_DIR="${PROJECT_ROOT}/10-common/include"
THIRDPARTY_LIB_DIR="${PROJECT_ROOT}/10-common/lib/releaselib/linux64/${BUILD_TYPE_LOWER}"

# 创建目录
mkdir -p ${THIRDPARTY_INCLUDE_DIR}
mkdir -p ${THIRDPARTY_LIB_DIR}

echo "========================================="
echo "Build Configuration:"
echo "  Type: ${BUILD_TYPE}"
echo "  Include: ${THIRDPARTY_INCLUDE_DIR}"
echo "  Library: ${THIRDPARTY_LIB_DIR}"
echo "========================================="

# ==================== 构建 jsoncpp ====================
echo "========================================="
echo "Building jsoncpp (${BUILD_TYPE})..."
echo "========================================="
cd b0-thirdparty/jsoncpp

echo "检查并修复 jsoncpp C++ 标准设置..."
# 强制设置 C++17
sed -i 's/CMAKE_CXX_STANDARD [0-9]\+/CMAKE_CXX_STANDARD 17/g' CMakeLists.txt
sed -i 's/-std=c++[0-9]\+/-std=c++17/g' CMakeLists.txt

rm -rf build-${BUILD_TYPE}
mkdir build-${BUILD_TYPE} && cd build-${BUILD_TYPE}

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_STANDARD_REQUIRED=ON \
      -DJSONCPP_WITH_TESTS=OFF \
      -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_OBJECT_LIBS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_INSTALL_PREFIX=/tmp/jsoncpp-install-${BUILD_TYPE} \
      ..

make -j$(nproc)
make install

# 拷贝文件
echo "Installing jsoncpp to ${THIRDPARTY_INCLUDE_DIR} and ${THIRDPARTY_LIB_DIR}"
rm -rf ${THIRDPARTY_INCLUDE_DIR}/json
cp -r /tmp/jsoncpp-install-${BUILD_TYPE}/include/json ${THIRDPARTY_INCLUDE_DIR}/
cp /tmp/jsoncpp-install-${BUILD_TYPE}/lib/libjsoncpp.a ${THIRDPARTY_LIB_DIR}/

# 验证
JSONCPP_STD=$(strings ${THIRDPARTY_LIB_DIR}/libjsoncpp.a | grep "std=c++" | head -1)
echo "jsoncpp 编译标准: $JSONCPP_STD"

cd ${PROJECT_ROOT}

# ==================== 构建 drogon ====================
echo ""
echo "========================================="
echo "Building drogon (${BUILD_TYPE})..."
echo "========================================="
cd b0-thirdparty/drogon
rm -rf build-${BUILD_TYPE}
mkdir build-${BUILD_TYPE} && cd build-${BUILD_TYPE}

# 只使用 CMAKE_PREFIX_PATH，让 CMake 自动查找
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_STANDARD_REQUIRED=ON \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_TESTING=OFF \
      -DBUILD_CTL=OFF \
      -DBUILD_ORM=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_PREFIX_PATH="/tmp/jsoncpp-install-${BUILD_TYPE}" \
      -DCMAKE_INSTALL_PREFIX=/tmp/drogon-install-${BUILD_TYPE} \
      .. 2>&1 | tee cmake-output.log

echo ""
echo "检查 drogon 使用的 jsoncpp："
if grep -q "/tmp/jsoncpp-install-${BUILD_TYPE}" CMakeCache.txt; then
    echo "✓ drogon 正确使用自定义 jsoncpp"
    grep "Jsoncpp" CMakeCache.txt | grep -v "^#"
else
    echo "✗ 错误: drogon 未使用自定义 jsoncpp"
    echo "CMake 输出已保存到 cmake-output.log"
    exit 1
fi

make -j$(nproc) 2>&1 | tee build-output.log
make install

# 拷贝文件
echo ""
echo "Installing drogon to ${THIRDPARTY_INCLUDE_DIR} and ${THIRDPARTY_LIB_DIR}"
rm -rf ${THIRDPARTY_INCLUDE_DIR}/drogon ${THIRDPARTY_INCLUDE_DIR}/trantor
cp -r /tmp/drogon-install-${BUILD_TYPE}/include/drogon ${THIRDPARTY_INCLUDE_DIR}/
cp -r /tmp/drogon-install-${BUILD_TYPE}/include/trantor ${THIRDPARTY_INCLUDE_DIR}/
cp /tmp/drogon-install-${BUILD_TYPE}/lib/libdrogon.a ${THIRDPARTY_LIB_DIR}/
cp /tmp/drogon-install-${BUILD_TYPE}/lib/libtrantor.a ${THIRDPARTY_LIB_DIR}/

# 验证
DROGON_STD=$(strings ${THIRDPARTY_LIB_DIR}/libdrogon.a | grep "std=c++" | head -1)
echo "drogon 编译标准: $DROGON_STD"

cd ${PROJECT_ROOT}

# 恢复系统的 jsoncpp（如果之前卸载了）
if [ "$REINSTALL_JSONCPP" = true ]; then
    echo ""
    echo "重新安装系统的 libjsoncpp-dev..."
    sudo apt-get install -y libjsoncpp-dev
fi

echo ""
echo "========================================="
echo "编译完成！"
echo "========================================="
echo "jsoncpp: $(echo $JSONCPP_STD | grep -o 'std=c++[0-9]*')"
echo "drogon:  $(echo $DROGON_STD | grep -o 'std=c++[0-9]*')"
echo "========================================="
ls -lh ${THIRDPARTY_LIB_DIR}/*.a
echo "========================================="
