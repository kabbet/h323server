cmake_minimum_required(VERSION 3.5)
project(mtlog VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 导出编译数据库
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 构建选项
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_EXECUTABLE "Build executable demo" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ============================================
# 智能路径检测：支持顶层编译和单独编译
# ============================================

# 检测是否为单独编译
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # 单独编译模式：计算相对路径到 10-common
    message(STATUS "mtlog: Standalone build mode")
    get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
    
    set(COMMON_DIR "${PROJECT_ROOT}/10-common")
    
    # 设置 Boost 路径
    set(BOOST_ROOT "${COMMON_DIR}")
    set(BOOST_INCLUDEDIR "${COMMON_DIR}/include")
    set(BOOST_LIBRARYDIR "${COMMON_DIR}/lib/releaselib/linux64")
    set(Boost_NO_SYSTEM_PATHS ON)
    set(Boost_NO_BOOST_CMAKE ON)
    
    # 添加包含路径和库路径
    include_directories(${COMMON_DIR}/include)
    link_directories(${COMMON_DIR}/lib/releaselib/linux64)
else()
    # 从顶层编译：使用父项目已经设置的路径
    message(STATUS "mtlog: Building as subdirectory")
    # BOOST_ROOT 等变量已由父 CMakeLists.txt 设置
    # include_directories 和 link_directories 也已设置
endif()

set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(Boost REQUIRED COMPONENTS filesystem thread log log_setup)

if(Boost_FOUND)
    message(STATUS "mtlog: Boost version: ${Boost_VERSION}")
    message(STATUS "mtlog: Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "mtlog: Boost libraries: ${Boost_LIBRARIES}")
endif()

# ============================================
# 源文件列表
# ============================================
set(${CMAKE_PROJECT_NAME}_SOURCES source/mtlog.cpp)

# ============================================
# 动态库
# ============================================
if(BUILD_SHARED_LIBS)
    add_library(${CMAKE_PROJECT_NAME}_shared SHARED ${${CMAKE_PROJECT_NAME}_SOURCES})
    set_target_properties(${CMAKE_PROJECT_NAME}_shared PROPERTIES
        OUTPUT_NAME ${CMAKE_PROJECT_NAME}
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        EXPORT_NAME shared  # 关键：指定导出名称
    )
    target_compile_definitions(${CMAKE_PROJECT_NAME}_shared 
        PUBLIC 
            BOOST_ALL_DYN_LINK
    )
    target_include_directories(${CMAKE_PROJECT_NAME}_shared
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(${CMAKE_PROJECT_NAME}_shared
        PUBLIC
            Boost::filesystem
            Boost::thread
            Boost::log
            Boost::log_setup
    )

    # 添加别名，方便子项目使用
    add_library(mtlog::shared ALIAS ${CMAKE_PROJECT_NAME}_shared)
endif()

# ============================================
# 静态库
# ============================================
if(BUILD_STATIC_LIBS)
    add_library(${CMAKE_PROJECT_NAME}_static STATIC ${${CMAKE_PROJECT_NAME}_SOURCES})
    set_target_properties(${CMAKE_PROJECT_NAME}_static PROPERTIES
        OUTPUT_NAME ${CMAKE_PROJECT_NAME}
        VERSION ${PROJECT_VERSION}
        EXPORT_NAME static  # 关键：指定导出名称
    )
    target_include_directories(${CMAKE_PROJECT_NAME}_static
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(${CMAKE_PROJECT_NAME}_static
        PUBLIC
            Boost::filesystem
            Boost::thread
            Boost::log
            Boost::log_setup
    )

    # 添加别名
    add_library(mtlog::static ALIAS ${CMAKE_PROJECT_NAME}_static)
endif()

# ============================================
# 安装配置
# ============================================

# 安装头文件（如果存在）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include)
    install(DIRECTORY include/ DESTINATION include)
endif()

# 准备要安装的目标列表
set(INSTALL_TARGETS)
if(BUILD_SHARED_LIBS)
    list(APPEND INSTALL_TARGETS ${CMAKE_PROJECT_NAME}_shared)
endif()
if(BUILD_STATIC_LIBS)
    list(APPEND INSTALL_TARGETS ${CMAKE_PROJECT_NAME}_static)
endif()

# 一次性安装所有目标并导出
if(INSTALL_TARGETS)
    install(TARGETS ${INSTALL_TARGETS}
        EXPORT mtlogTargets
        LIBRARY DESTINATION .
        ARCHIVE DESTINATION .
        RUNTIME DESTINATION .
    )

    # 导出目标文件
    install(EXPORT mtlogTargets
        FILE mtlogTargets.cmake
        NAMESPACE mtlog::
        DESTINATION cmake/mtlog
    )
endif()

# ============================================
# 生成配置文件
# ============================================
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/mtlogConfig.cmake
    INSTALL_DESTINATION cmake/mtlog
)

# 生成版本文件
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/mtlogConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# 安装配置文件
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/mtlogConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/mtlogConfigVersion.cmake
    DESTINATION cmake/mtlog
)

# ============================================
# 打印配置信息
# ============================================
message(STATUS "=== mtlog Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "Build static libs: ${BUILD_STATIC_LIBS}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "Common dir: ${COMMON_DIR}")
endif()
message(STATUS "===========================")
