# project/CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(MTCBB_Project VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ==================== 构建类型相关配置 ====================
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(BUILD_TYPE_LOWER "debug")
else()
    set(BUILD_TYPE_LOWER "release")
endif()

# 路径设置（第三方库现在也区分 debug/release）
set(COMMON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/10-common/include)
set(COMMON_LIB_DIR ${CMAKE_SOURCE_DIR}/10-common/lib/releaselib/linux64/${BUILD_TYPE_LOWER})
set(COMMON_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/10-common/lib/locallib/linux64/${BUILD_TYPE_LOWER})

file(MAKE_DIRECTORY ${COMMON_OUTPUT_DIR})

# ==================== 系统依赖 ====================
find_package(Boost 1.70 REQUIRED COMPONENTS log log_setup system thread filesystem)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found!")
endif()

# drogon 需要的系统库
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

find_library(BROTLIENC_LIB NAMES brotlienc)
find_library(BROTLIDEC_LIB NAMES brotlidec)
find_library(BROTLICOMMON_LIB NAMES brotlicommon)
find_library(UUID_LIB NAMES uuid)
find_library(SQLITE3_LIB NAMES sqlite3)

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Third-party Lib Dir: ${COMMON_LIB_DIR}")

# ==================== 创建 common_interface ====================
add_library(common_interface INTERFACE)
target_include_directories(common_interface 
    INTERFACE ${COMMON_INCLUDE_DIR}
)

# ==================== 导入第三方库 ====================
# jsoncpp
set(JSONCPP_LIB ${COMMON_LIB_DIR}/libjsoncpp.a)
if(EXISTS ${JSONCPP_LIB})
    message(STATUS "Found jsoncpp: ${JSONCPP_LIB}")
    
    add_library(jsoncpp_lib STATIC IMPORTED GLOBAL)
    set_target_properties(jsoncpp_lib PROPERTIES
        IMPORTED_LOCATION ${JSONCPP_LIB}
    )
    target_include_directories(jsoncpp_lib INTERFACE ${COMMON_INCLUDE_DIR})
else()
    message(WARNING "jsoncpp library not found at ${JSONCPP_LIB}")
endif()

# trantor (必须在 drogon 之前)
set(TRANTOR_LIB ${COMMON_LIB_DIR}/libtrantor.a)
if(EXISTS ${TRANTOR_LIB})
    message(STATUS "Found trantor: ${TRANTOR_LIB}")
    
    add_library(trantor STATIC IMPORTED GLOBAL)
    set_target_properties(trantor PROPERTIES
        IMPORTED_LOCATION ${TRANTOR_LIB}
    )
    target_include_directories(trantor INTERFACE ${COMMON_INCLUDE_DIR})
    
    # trantor 依赖 OpenSSL
    target_link_libraries(trantor
        INTERFACE
            OpenSSL::SSL
            OpenSSL::Crypto
    )
else()
    message(WARNING "trantor library not found at ${TRANTOR_LIB}")
endif()

set(DROGON_LIB ${COMMON_LIB_DIR}/libdrogon.a)
if(EXISTS ${DROGON_LIB})
    message(STATUS "Found drogon: ${DROGON_LIB}")

    add_library(drogon STATIC IMPORTED)
    set_target_properties(drogon PROPERTIES
        IMPORTED_LOCATION ${DROGON_LIB}
    )
    target_include_directories(drogon
        INTERFACE ${COMMON_INCLUDE_DIR}
    )

    # 查找 hiredis 库
    find_library(HIREDIS_LIB NAMES hiredis)
    if(NOT HIREDIS_LIB)
        message(WARNING "hiredis not found, if you use Redis features, install it with: sudo apt-get install libhiredis-dev")
    endif()

    # drogon 需要链接所有依赖
    target_link_libraries(drogon
        INTERFACE
            jsoncpp_lib          # 添加这行 - jsoncpp 必须在前面
            trantor
            OpenSSL::SSL
            OpenSSL::Crypto
            ZLIB::ZLIB
            ${BROTLIENC_LIB}
            ${BROTLIDEC_LIB}
            ${BROTLICOMMON_LIB}
            ${UUID_LIB}
            ${SQLITE3_LIB}
            ${HIREDIS_LIB}       # 添加这行
            pthread
            dl
    )
else()
    message(FATAL_ERROR
        "drogon library not found at ${DROGON_LIB}\n"
        "Please run: ./build-thirdparty.sh ${CMAKE_BUILD_TYPE}"
    )
endif()

# ==================== 项目模块 ====================
add_subdirectory(20-mtcbb/mtlog)
add_subdirectory(20-mtcbb/httpserver)

# ==================== 打印配置信息 ====================
message(STATUS "========================================")
message(STATUS "Project Configuration:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Common Include: ${COMMON_INCLUDE_DIR}")
message(STATUS "  Third-party Lib: ${COMMON_LIB_DIR}")
message(STATUS "  Project Lib Output: ${COMMON_OUTPUT_DIR}")
message(STATUS "========================================")
