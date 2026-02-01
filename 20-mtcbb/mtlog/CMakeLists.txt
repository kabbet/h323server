# 收集源文件
cmake_minimum_required(VERSION 3.14)
project(mtlog VERSION 1.0.0)


file(GLOB_RECURSE MTLOG_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "Building mtlog as standalone project")
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(BUILD_TYPE_LOWER "debug")
    else()
        set(BUILD_TYPE_LOWER "release")
    endif()

    set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    set(COMMON_INCLUDE_DIR ${PROJECT_ROOT}/10-common/include)
    set(COMMON_OUTPUT_DIR ${PROJECT_ROOT}/10-common/lib/locallib/linux64/${BUILD_TYPE_LOWER})

    find_package(Boost 1.70 REQUIRED COMPONENTS log log_setup system thread filesystem)
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost not found!")
    endif()
endif()

# ==================== 创建对象库（只编译一次） ====================
add_library(mtlog_objects OBJECT ${MTLOG_SOURCES})

target_include_directories(mtlog_objects 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${COMMON_INCLUDE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(mtlog_objects
    PUBLIC
        Boost::log
        Boost::log_setup
        Boost::system
        Boost::thread
        Boost::filesystem
)

target_compile_options(mtlog_objects PRIVATE
    -Wall
    -fPIC  # 对象文件需要 PIC，因为动态库需要
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

# ==================== 从对象库创建静态库 ====================
add_library(mtlog_static STATIC $<TARGET_OBJECTS:mtlog_objects>)
set_target_properties(mtlog_static PROPERTIES OUTPUT_NAME "mtlog")
target_link_libraries(mtlog_static PUBLIC mtlog_objects)

# ==================== 从对象库创建动态库 ====================
add_library(mtlog_shared SHARED $<TARGET_OBJECTS:mtlog_objects>)
set_target_properties(mtlog_shared PROPERTIES 
    OUTPUT_NAME "mtlog"
)
target_link_libraries(mtlog_shared PUBLIC mtlog_objects)

# ==================== 拷贝库文件 ====================
add_custom_command(TARGET mtlog_static POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:mtlog_static>
        ${COMMON_OUTPUT_DIR}/libmtlog.a
    COMMENT "Copying libmtlog.a to ${COMMON_OUTPUT_DIR}"
)

add_custom_command(TARGET mtlog_shared POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:mtlog_shared>
        ${COMMON_OUTPUT_DIR}/libmtlog.so
    COMMENT "Copying libmtlog.so to ${COMMON_OUTPUT_DIR}"
)

# 默认别名
add_library(mtlog ALIAS mtlog_static)
