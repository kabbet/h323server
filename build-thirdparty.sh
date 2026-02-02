#!/bin/bash

set -e

BUILD_TYPE=${1:-Debug}
PROJECT_ROOT=$(pwd)

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
echo "正在检查 jsoncpp 的 C++ 标准设置..."
echo "========================================"

# 检查并替换 CMAKE_CXX_STANDARD 11
if grep -q "CMAKE_CXX_STANDARD 11" CMakeLists.txt; then
    echo "✓ 找到 CMAKE_CXX_STANDARD 11，正在替换为 17..."
    sed -i 's/CMAKE_CXX_STANDARD 11/CMAKE_CXX_STANDARD 17/g' CMakeLists.txt
    echo "  替换成功"
else
    echo "✗ 未找到 CMAKE_CXX_STANDARD 11"
fi

# 检查并替换 -std=c++11
if grep -q "\-std=c++11" CMakeLists.txt; then
    echo "✓ 找到 -std=c++11，正在替换为 -std=c++17..."
    sed -i 's/-std=c++11/-std=c++17/g' CMakeLists.txt
    echo "  替换成功"
else
    echo "✗ 未找到 -std=c++11"
fi

echo "========================================"
echo "检查完成，当前 C++ 标准设置："
grep -n "CXX_STANDARD\|std=c++" CMakeLists.txt | head -5

rm -rf build-${BUILD_TYPE}
mkdir build-${BUILD_TYPE} && cd build-${BUILD_TYPE}

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DJSONCPP_WITH_TESTS=OFF \
      -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_OBJECT_LIBS=OFF \
      -DCMAKE_INSTALL_PREFIX=/tmp/jsoncpp-install-${BUILD_TYPE} \
      ..

make -j$(nproc)
make install

# 拷贝头文件到统一位置（只在第一次构建时）
if [ ! -d "${THIRDPARTY_INCLUDE_DIR}/json" ]; then
    echo "Installing jsoncpp headers to ${THIRDPARTY_INCLUDE_DIR}"
    cp -r /tmp/jsoncpp-install-${BUILD_TYPE}/include/json ${THIRDPARTY_INCLUDE_DIR}/
fi

# 拷贝库文件到对应的 debug/release 目录
echo "Installing jsoncpp library to ${THIRDPARTY_LIB_DIR}"
cp /tmp/jsoncpp-install-${BUILD_TYPE}/lib/libjsoncpp.a ${THIRDPARTY_LIB_DIR}/

# 清理临时目录
rm -rf /tmp/jsoncpp-install-${BUILD_TYPE}

cd ${PROJECT_ROOT}

# ==================== 构建 drogon ====================
echo "========================================="
echo "Building drogon (${BUILD_TYPE})..."
echo "========================================="
cd b0-thirdparty/drogon
rm -rf build-${BUILD_TYPE}
mkdir build-${BUILD_TYPE} && cd build-${BUILD_TYPE}

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_TESTING=OFF \
      -DBUILD_DROGON_SHARED=OFF \
      -DBUILD_CTL=OFF \
      -DBUILD_ORM=ON \
      -DCMAKE_INSTALL_PREFIX=/tmp/drogon-install-${BUILD_TYPE} \
      ..

make -j$(nproc)
make install

# 拷贝头文件到统一位置（只在第一次构建时）
if [ ! -d "${THIRDPARTY_INCLUDE_DIR}/drogon" ]; then
    echo "Installing drogon headers to ${THIRDPARTY_INCLUDE_DIR}"
    cp -r /tmp/drogon-install-${BUILD_TYPE}/include/drogon ${THIRDPARTY_INCLUDE_DIR}/
    cp -r /tmp/drogon-install-${BUILD_TYPE}/include/trantor ${THIRDPARTY_INCLUDE_DIR}/
fi

# 拷贝库文件到对应的 debug/release 目录
echo "Installing drogon libraries to ${THIRDPARTY_LIB_DIR}"
cp /tmp/drogon-install-${BUILD_TYPE}/lib/libdrogon.a ${THIRDPARTY_LIB_DIR}/
cp /tmp/drogon-install-${BUILD_TYPE}/lib/libtrantor.a ${THIRDPARTY_LIB_DIR}/

# 清理临时目录
rm -rf /tmp/drogon-install-${BUILD_TYPE}

cd ${PROJECT_ROOT}

echo "========================================="
echo "Third-party libraries built successfully!"
echo "========================================="
echo "Headers installed to: ${THIRDPARTY_INCLUDE_DIR}"
echo "  - json/"
echo "  - drogon/"
echo "  - trantor/"
echo ""
echo "Libraries installed to: ${THIRDPARTY_LIB_DIR}"
ls -lh ${THIRDPARTY_LIB_DIR}/*.a 2>/dev/null || echo "  (no libraries found)"
echo "========================================="
