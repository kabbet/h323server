# project/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(h323server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_DEMO "Build demo executable" OFF)
option(BUILD_HTTPSERVER "Build httpserver executable" ON)

# ============================================
# 配置 10-common 的头文件和库文件路径
# ============================================

# 定义 10-common 的路径
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/10-common)

# 设置全局 Boost 路径（所有子项目可用）
set(BOOST_ROOT ${COMMON_DIR} CACHE PATH "Boost root directory")
set(BOOST_INCLUDEDIR ${COMMON_DIR}/include CACHE PATH "Boost include directory")
set(BOOST_LIBRARYDIR ${COMMON_DIR}/lib/releaselib/linux64 CACHE PATH "Boost library directory")
set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "Do not search system paths for Boost")
set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "Do not use Boost's CMake config")
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static libraries")

include_directories(${COMMON_DIR}/include)
link_directories(
    ${COMMON_DIR}/lib/locallib/linux64/Debug
    ${COMMON_DIR}/lib/releaselib/linux64
)

# ============================================
# 添加子项目
# ============================================

# 自动查找 20-mtcbb 下的所有子目录
file(GLOB SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/20-mtcbb 
     ${CMAKE_CURRENT_SOURCE_DIR}/20-mtcbb/*)


add_subdirectory(20-mtcbb/mtlog)
if(BUILD_DEMO)
    add_subdirectory(20-mtcbb/demo)
endif()

if(BUILD_HTTPSERVER)
    add_subdirectory(20-mtcbb/httpserver)
endif()

