# 测试项目 CMakeLists.txt - 自动查找 Models 文件
cmake_minimum_required(VERSION 3.14)
project(UserServiceTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# 构建类型
# ============================================================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(BUILD_TYPE_LOWER "debug")
else()
    set(BUILD_TYPE_LOWER "release")
endif()

message(STATUS "Test Build Type: ${CMAKE_BUILD_TYPE} (${BUILD_TYPE_LOWER})")

# ============================================================================
# 项目路径配置
# ============================================================================

set(HTTPSERVER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "httpserver root directory")
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." CACHE PATH "Project root directory")

# 第三方库路径
set(COMMON_INCLUDE_DIR ${PROJECT_ROOT}/10-common/include)
set(COMMON_LIB_DIR ${PROJECT_ROOT}/10-common/lib/releaselib/linux64/${BUILD_TYPE_LOWER})
set(COMMON_LOCAL_LIB_DIR ${PROJECT_ROOT}/10-common/lib/locallib/linux64/${BUILD_TYPE_LOWER})

message(STATUS "========================================")
message(STATUS "Project Paths:")
message(STATUS "  HTTPSERVER_ROOT: ${HTTPSERVER_ROOT}")
message(STATUS "  PROJECT_ROOT: ${PROJECT_ROOT}")
message(STATUS "  COMMON_INCLUDE: ${COMMON_INCLUDE_DIR}")
message(STATUS "  COMMON_LIB: ${COMMON_LIB_DIR}")
message(STATUS "========================================")

# ============================================================================
# 查找系统依赖
# ============================================================================

# Boost
find_package(Boost 1.70 REQUIRED COMPONENTS 
    unit_test_framework
    log log_setup system thread filesystem
)

# PostgreSQL
set(POSTGRESQL_ROOT "$ENV{PG_HOME}")
if(EXISTS "${POSTGRESQL_ROOT}/bin/pg_config")
    execute_process(
        COMMAND ${POSTGRESQL_ROOT}/bin/pg_config --includedir
        OUTPUT_VARIABLE PG_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${POSTGRESQL_ROOT}/bin/pg_config --libdir
        OUTPUT_VARIABLE PG_LIB_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(PostgreSQL_INCLUDE_DIRS "${PG_INCLUDE_DIR}")
    set(PostgreSQL_LIBRARIES "${PG_LIB_DIR}/libpq.so")
    set(PostgreSQL_FOUND TRUE)
    message(STATUS "PostgreSQL found: ${PG_INCLUDE_DIR}")
else()
    message(WARNING "PostgreSQL not found")
endif()

# OpenSSL, ZLIB 等
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_library(BROTLIENC_LIB NAMES brotlienc)
find_library(BROTLIDEC_LIB NAMES brotlidec)
find_library(BROTLICOMMON_LIB NAMES brotlicommon)
find_library(UUID_LIB NAMES uuid)
find_library(SQLITE3_LIB NAMES sqlite3)

# Hiredis
set(HIREDIS_ROOT "$ENV{HIREDIS_HOME}")
if(EXISTS "${HIREDIS_ROOT}")
    set(HIREDIS_INCLUDE_DIR "${HIREDIS_ROOT}/include")
    set(HIREDIS_LIBRARY "${HIREDIS_ROOT}/lib/libhiredis.so")
    message(STATUS "Hiredis found: ${HIREDIS_ROOT}")
endif()

# ============================================================================
# 创建 common_interface
# ============================================================================

add_library(common_interface INTERFACE)
target_include_directories(common_interface 
    INTERFACE ${COMMON_INCLUDE_DIR}
)

# ============================================================================
# 导入第三方库
# ============================================================================

# jsoncpp
set(JSONCPP_LIB ${COMMON_LIB_DIR}/libjsoncpp.a)
if(EXISTS ${JSONCPP_LIB})
    message(STATUS "Found jsoncpp: ${JSONCPP_LIB}")
    add_library(jsoncpp_lib STATIC IMPORTED)
    set_target_properties(jsoncpp_lib PROPERTIES IMPORTED_LOCATION ${JSONCPP_LIB})
    target_include_directories(jsoncpp_lib INTERFACE ${COMMON_INCLUDE_DIR})
else()
    message(WARNING "jsoncpp not found at ${JSONCPP_LIB}")
endif()

# trantor
set(TRANTOR_LIB ${COMMON_LIB_DIR}/libtrantor.a)
if(EXISTS ${TRANTOR_LIB})
    message(STATUS "Found trantor: ${TRANTOR_LIB}")
    add_library(trantor STATIC IMPORTED)
    set_target_properties(trantor PROPERTIES IMPORTED_LOCATION ${TRANTOR_LIB})
    target_include_directories(trantor INTERFACE ${COMMON_INCLUDE_DIR})
    target_link_libraries(trantor INTERFACE OpenSSL::SSL OpenSSL::Crypto)
else()
    message(FATAL_ERROR "trantor not found at ${TRANTOR_LIB}")
endif()

# drogon
set(DROGON_LIB ${COMMON_LIB_DIR}/libdrogon.a)
if(EXISTS ${DROGON_LIB})
    message(STATUS "Found drogon: ${DROGON_LIB}")
    add_library(drogon STATIC IMPORTED)
    set_target_properties(drogon PROPERTIES IMPORTED_LOCATION ${DROGON_LIB})
    target_include_directories(drogon INTERFACE ${COMMON_INCLUDE_DIR})
    target_link_libraries(drogon INTERFACE
        jsoncpp_lib trantor
        OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB
        ${BROTLIENC_LIB} ${BROTLIDEC_LIB} ${BROTLICOMMON_LIB}
        ${UUID_LIB} ${SQLITE3_LIB} ${HIREDIS_LIBRARY}
        pthread dl
    )
else()
    message(FATAL_ERROR "drogon not found at ${DROGON_LIB}")
endif()

# mtlog
set(MTLOG_LIB ${COMMON_LOCAL_LIB_DIR}/libmtlog.a)
if(EXISTS ${MTLOG_LIB})
    message(STATUS "Found mtlog: ${MTLOG_LIB}")
    add_library(mtlog STATIC IMPORTED)
    set_target_properties(mtlog PROPERTIES IMPORTED_LOCATION ${MTLOG_LIB})
    target_include_directories(mtlog INTERFACE ${PROJECT_ROOT}/20-mtcbb/mtlog/source)
    target_link_libraries(mtlog INTERFACE
        Boost::log Boost::log_setup Boost::system Boost::thread Boost::filesystem
    )
else()
    message(WARNING "mtlog not found at ${MTLOG_LIB}")
endif()

# ============================================================================
# 包含目录
# ============================================================================

include_directories(
    ${HTTPSERVER_ROOT}/include
    ${HTTPSERVER_ROOT}/include/adapters
    ${HTTPSERVER_ROOT}/include/controllers
    ${HTTPSERVER_ROOT}/include/handlers
    ${HTTPSERVER_ROOT}/include/infrastructure
    ${HTTPSERVER_ROOT}/include/interfaces
    ${HTTPSERVER_ROOT}/include/models
    ${HTTPSERVER_ROOT}/include/repositories
    ${HTTPSERVER_ROOT}/include/services
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${Boost_INCLUDE_DIRS}
)

# ============================================================================
# 自动查找 Drogon ORM Models 实现文件
# ============================================================================

message(STATUS "========================================")
message(STATUS "Searching for Drogon ORM Models...")
message(STATUS "========================================")

# 可能的 Models 位置
set(MODELS_SEARCH_PATHS
    "${HTTPSERVER_ROOT}/models"
    "${HTTPSERVER_ROOT}/source/models"
    "${HTTPSERVER_ROOT}/build/models"
)

set(MODELS_SOURCES "")

foreach(SEARCH_PATH ${MODELS_SEARCH_PATHS})
    if(EXISTS "${SEARCH_PATH}")
        message(STATUS "Checking: ${SEARCH_PATH}")
        
        # 查找 Users.cc 和 UserTokens.cc
        file(GLOB FOUND_MODELS 
            "${SEARCH_PATH}/Users.cc"
            "${SEARCH_PATH}/Users.cpp"
            "${SEARCH_PATH}/UserTokens.cc"
            "${SEARCH_PATH}/UserTokens.cpp"
        )
        
        if(FOUND_MODELS)
            list(APPEND MODELS_SOURCES ${FOUND_MODELS})
            message(STATUS "  ✓ Found models:")
            foreach(MODEL_FILE ${FOUND_MODELS})
                get_filename_component(MODEL_NAME ${MODEL_FILE} NAME)
                message(STATUS "    - ${MODEL_NAME}")
            endforeach()
        else()
            message(STATUS "  ✗ No model files found")
        endif()
    endif()
endforeach()

if(NOT MODELS_SOURCES)
    message(WARNING 
        "========================================\n"
        "WARNING: Drogon ORM Models not found!\n"
        "========================================\n"
        "Searched in:\n"
        "  - ${HTTPSERVER_ROOT}/models\n"
        "  - ${HTTPSERVER_ROOT}/source/models\n"
        "  - ${HTTPSERVER_ROOT}/build/models\n"
        "\n"
        "If linking fails with 'undefined reference' errors,\n"
        "you need to:\n"
        "\n"
        "1. Generate models with drogon_ctl:\n"
        "   cd ${HTTPSERVER_ROOT}\n"
        "   drogon_ctl create model myapp\n"
        "\n"
        "2. Or manually add them to CMakeLists.txt:\n"
        "   set(MANUAL_MODELS_SOURCES\n"
        "       \${HTTPSERVER_ROOT}/path/to/Users.cc\n"
        "       \${HTTPSERVER_ROOT}/path/to/UserTokens.cc\n"
        "   )\n"
        "========================================\n"
    )
else()
    message(STATUS "✓ Found ${CMAKE_MATCH_COUNT} model files")
endif()

# ============================================================================
# 源文件
# ============================================================================

# 测试文件
set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(TEST_SOURCES
    ${TEST_DIR}/test_user_service.cpp
)

if(NOT EXISTS "${TEST_DIR}/test_user_service.cpp")
    message(FATAL_ERROR "Test source file not found: ${TEST_DIR}/test_user_service.cpp")
endif()

# 项目源文件 = UserService + Models
set(PROJECT_SOURCES
    ${HTTPSERVER_ROOT}/source/services/UserService.cpp
    ${MODELS_SOURCES}  # ← 自动找到的 Models 文件
)

# 如果自动查找失败，可以手动指定
set(MANUAL_MODELS_SOURCES "" CACHE STRING "Manually specify model sources if auto-detection fails")
if(MANUAL_MODELS_SOURCES)
    message(STATUS "Using manually specified models: ${MANUAL_MODELS_SOURCES}")
    list(APPEND PROJECT_SOURCES ${MANUAL_MODELS_SOURCES})
endif()

message(STATUS "")
message(STATUS "Source files to compile:")
foreach(SRC ${PROJECT_SOURCES})
    get_filename_component(SRC_NAME ${SRC} NAME)
    message(STATUS "  - ${SRC_NAME}")
endforeach()
message(STATUS "")

# ============================================================================
# 创建测试可执行文件
# ============================================================================

add_executable(user_service_tests
    ${PROJECT_SOURCES}
    ${TEST_SOURCES}
)

# ============================================================================
# 链接库
# ============================================================================

target_link_libraries(user_service_tests
    PRIVATE
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        common_interface
        drogon
        jsoncpp_lib
        trantor
        ${PostgreSQL_LIBRARIES}
        pthread
)

# ============================================================================
# 编译选项
# ============================================================================

target_compile_options(user_service_tests PRIVATE
    -Wall
    -Wextra
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# ============================================================================
# 测试配置
# ============================================================================

enable_testing()

add_test(
    NAME UserServiceTests
    COMMAND user_service_tests --log_level=all --report_level=detailed
)

# ============================================================================
# 输出目录
# ============================================================================

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================================================
# 打印配置信息
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Test Configuration Summary")
message(STATUS "========================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Paths:")
message(STATUS "  httpserver: ${HTTPSERVER_ROOT}")
message(STATUS "  Project root: ${PROJECT_ROOT}")
message(STATUS "  Third-party lib: ${COMMON_LIB_DIR}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  Boost: ${Boost_VERSION}")
message(STATUS "  Drogon: ${DROGON_LIB}")
message(STATUS "  PostgreSQL: ${PostgreSQL_FOUND}")
message(STATUS "")
message(STATUS "Models:")
message(STATUS "  Found: ${MODELS_SOURCES}")
message(STATUS "")
message(STATUS "Output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
message(STATUS "")
