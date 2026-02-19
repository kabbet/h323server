# 20-mtcbb/httpserver/CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(httpserver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== 检测是否独立编译 ====================
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "Building httpserver as standalone project")
    
    # 设置构建类型
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(BUILD_TYPE_LOWER "debug")
    else()
        set(BUILD_TYPE_LOWER "release")
    endif()

    # 设置路径（第三方库现在也区分 debug/release）
    set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    set(COMMON_INCLUDE_DIR ${PROJECT_ROOT}/10-common/include)
    set(COMMON_LIB_DIR ${PROJECT_ROOT}/10-common/lib/releaselib/linux64/${BUILD_TYPE_LOWER})
    set(COMMON_LOCAL_LIB_DIR ${PROJECT_ROOT}/10-common/lib/locallib/linux64/${BUILD_TYPE_LOWER})
    set(COMMON_BIN_OUTPUT_DIR ${PROJECT_ROOT}/10-common/version/bin/${BUILD_TYPE_LOWER})
    
    file(MAKE_DIRECTORY ${COMMON_BIN_OUTPUT_DIR})

    # ==================== 查找系统依赖 ====================
    find_package(Boost 1.70 REQUIRED COMPONENTS log log_setup system thread filesystem)
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost not found!")
    endif()


    set(POSTGRESQL_ROOT "$ENV{PG_HOME}")

    # 验证 PostgreSQL 安装
    if(EXISTS "${POSTGRESQL_ROOT}/bin/pg_config")
        # 使用 pg_config 获取准确路径
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

        message(STATUS "PostgreSQL found via pg_config")
        message(STATUS "  Include: ${PG_INCLUDE_DIR}")
        message(STATUS "  Lib: ${PG_LIB_DIR}")

        # 设置 PostgreSQL 变量
        set(PostgreSQL_INCLUDE_DIR "${PG_INCLUDE_DIR}")
        set(PostgreSQL_LIBRARY "${PG_LIB_DIR}/libpq.so")
        set(PostgreSQL_TYPE_INCLUDE_DIR "${PG_INCLUDE_DIR}/server")

        # 手动设置为已找到
        set(PostgreSQL_FOUND TRUE)
        set(PostgreSQL_INCLUDE_DIRS "${PG_INCLUDE_DIR}")
        set(PostgreSQL_LIBRARIES "${PG_LIB_DIR}/libpq.so")

    else()  # ← 修复：添加括号
        message(FATAL_ERROR
            "PostgreSQL not found!\n"
            "Checked: ${POSTGRESQL_ROOT}/bin/pg_config\n"
            "Please verify stow installation: ls -la ~/.slocal/bin/pg_config"
        )
    endif()


    # 查找 drogon 需要的系统库
    find_package(OpenSSL REQUIRED)
    find_package(ZLIB REQUIRED)
    
    find_library(BROTLIENC_LIB NAMES brotlienc)
    find_library(BROTLIDEC_LIB NAMES brotlidec)
    find_library(BROTLICOMMON_LIB NAMES brotlicommon)
    find_library(UUID_LIB NAMES uuid)
    find_library(SQLITE3_LIB NAMES sqlite3)
    find_package(PostgreSQL REQUIRED)

    
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
        
        add_library(jsoncpp_lib STATIC IMPORTED)
        set_target_properties(jsoncpp_lib PROPERTIES
            IMPORTED_LOCATION ${JSONCPP_LIB}
        )
        target_include_directories(jsoncpp_lib 
            INTERFACE ${COMMON_INCLUDE_DIR}
        )
    else()
        message(FATAL_ERROR 
            "jsoncpp library not found at ${JSONCPP_LIB}\n"
            "Please run: ./build-thirdparty.sh ${CMAKE_BUILD_TYPE}"
        )
    endif()
    
    # trantor (必须先导入，因为 drogon 依赖它)
    set(TRANTOR_LIB ${COMMON_LIB_DIR}/libtrantor.a)
    if(EXISTS ${TRANTOR_LIB})
        message(STATUS "Found trantor: ${TRANTOR_LIB}")
        
        add_library(trantor STATIC IMPORTED)
        set_target_properties(trantor PROPERTIES
            IMPORTED_LOCATION ${TRANTOR_LIB}
        )
        target_include_directories(trantor 
            INTERFACE ${COMMON_INCLUDE_DIR}
        )
        
        # trantor 需要链接 OpenSSL
        target_link_libraries(trantor
            INTERFACE
                OpenSSL::SSL
                OpenSSL::Crypto
        )
    else()
        message(FATAL_ERROR "trantor library not found at ${TRANTOR_LIB}")
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
    
         set(HIREDIS_ROOT "$ENV{HIREDIS_HOME}")

         # 验证 HIREDIS 安装
         if(EXISTS "${HIREDIS_ROOT}")
             # 设置 HIREDIS 变量
             set(HIREDIS_INCLUDE_DIR "${HIREDIS_ROOT}/include")
             set(HIREDIS_LIBRARY "${HIREDIS_ROOT}/lib/libhiredis.so")

         else()
             message(FATAL_ERROR
                 "HIREDIS not found!\n"
                 "Checked: ${HIREDIS_ROOT}/\n"
             )
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
                ${HIREDIS_LIBRARY}
                pthread
                dl
        )
    else()
        message(FATAL_ERROR
            "drogon library not found at ${DROGON_LIB}\n"
            "Please run: ./build-thirdparty.sh ${CMAKE_BUILD_TYPE}"
        )
    endif()

    # ==================== 导入 mtlog ====================
    set(MTLOG_LIB ${COMMON_LOCAL_LIB_DIR}/libmtlog.a)
    
    if(EXISTS ${MTLOG_LIB})
        message(STATUS "Found mtlog: ${MTLOG_LIB}")
        
        add_library(mtlog STATIC IMPORTED)
        set_target_properties(mtlog PROPERTIES
            IMPORTED_LOCATION ${MTLOG_LIB}
        )
        
        target_include_directories(mtlog 
            INTERFACE ${PROJECT_ROOT}/20-mtcbb/mtlog/source
        )
        
        target_link_libraries(mtlog
            INTERFACE
                Boost::log
                Boost::log_setup
                Boost::system
                Boost::thread
                Boost::filesystem
        )
    else()
        message(FATAL_ERROR "mtlog library not found at ${MTLOG_LIB}")
    endif()
    
    message(STATUS "========================================")
    message(STATUS "All dependencies loaded successfully")
    message(STATUS "========================================")
endif()

# ==================== httpserver 可执行文件 ====================
file(GLOB_RECURSE HTTPSERVER_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

if(NOT HTTPSERVER_SOURCES)
    message(FATAL_ERROR "No source files found in ${CMAKE_CURRENT_SOURCE_DIR}/source/")
endif()

add_executable(httpserver ${HTTPSERVER_SOURCES})

target_include_directories(httpserver 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/adapters
        ${CMAKE_CURRENT_SOURCE_DIR}/include/controllers
        ${CMAKE_CURRENT_SOURCE_DIR}/include/handlers
        ${CMAKE_CURRENT_SOURCE_DIR}/include/infrastructure
        ${CMAKE_CURRENT_SOURCE_DIR}/include/interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/include/models
        ${CMAKE_CURRENT_SOURCE_DIR}/include/repositories
        ${CMAKE_CURRENT_SOURCE_DIR}/include/services
        ${CMAKE_CURRENT_SOURCE_DIR}/include/filters
)

# 注意链接顺序：从高层到低层
target_link_libraries(httpserver 
    PRIVATE 
        common_interface
        mtlog
        drogon
        jsoncpp_lib
        ${PostgreSQL_LIBRARIES}
)

target_compile_options(httpserver PRIVATE
    -Wall
    -Wextra
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# ==================== 安装/拷贝 ====================
if(DEFINED COMMON_BIN_OUTPUT_DIR)
    add_custom_command(TARGET httpserver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
            $<TARGET_FILE:httpserver>
            ${COMMON_BIN_OUTPUT_DIR}/httpserver
        COMMENT "Copying httpserver to ${COMMON_BIN_OUTPUT_DIR}"
    )
    
    message(STATUS "httpserver will be copied to: ${COMMON_BIN_OUTPUT_DIR}")
endif()

# ==================== 打印配置信息 ====================
message(STATUS "========================================")
message(STATUS "httpserver Configuration Summary:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Third-party Lib: ${COMMON_LIB_DIR}")
message(STATUS "  Local Lib: ${COMMON_LOCAL_LIB_DIR}")
if(DEFINED COMMON_BIN_OUTPUT_DIR)
    message(STATUS "  Output: ${COMMON_BIN_OUTPUT_DIR}/httpserver")
endif()
message(STATUS "========================================")
