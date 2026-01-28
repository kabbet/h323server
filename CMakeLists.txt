# project/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ============================================
# 配置 10-common 的头文件和库文件路径
# ============================================

# 定义 10-common 的路径
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/10-common)

# 全局添加头文件搜索路径（所有子项目都能访问）
include_directories(${COMMON_DIR}/include)

# 全局添加库文件搜索路径
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

foreach(subdir ${SUBDIRS})
    set(full_path ${CMAKE_CURRENT_SOURCE_DIR}/20-mtcbb/${subdir})
    # 检查是否是目录且包含 CMakeLists.txt
    if(IS_DIRECTORY ${full_path} AND EXISTS ${full_path}/CMakeLists.txt)
        message(STATUS "Adding subdirectory: 20-mtcbb/${subdir}")
        add_subdirectory(20-mtcbb/${subdir})
    endif()
endforeach()
