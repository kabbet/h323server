# 20-mtcbb/demo/CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== 检测是否独立编译 ====================
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "Building demo as standalone project")
    
    # 设置构建类型
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(BUILD_TYPE_LOWER "debug")
    else()
        set(BUILD_TYPE_LOWER "release")
    endif()

    # 设置路径
    set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    set(COMMON_INCLUDE_DIR ${PROJECT_ROOT}/10-common/include)
    set(COMMON_LIB_DIR ${PROJECT_ROOT}/10-common/lib/releaselib/linux64/${BUILD_TYPE_LOWER})
    set(COMMON_LOCAL_LIB_DIR ${PROJECT_ROOT}/10-common/lib/locallib/linux64/${BUILD_TYPE_LOWER})
    set(COMMON_BIN_OUTPUT_DIR ${PROJECT_ROOT}/10-common/version/bin/${BUILD_TYPE_LOWER})

    # 查找 Boost
    find_package(Boost 1.70 REQUIRED COMPONENTS log log_setup system thread filesystem)
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost not found!")
    endif()

    # ==================== 创建 common_interface ====================
    add_library(common_interface INTERFACE)
    target_include_directories(common_interface 
        INTERFACE 
            ${COMMON_INCLUDE_DIR}
    )
    
    # 链接 10-common 下的预编译库（如果有）
    file(GLOB COMMON_LIBS ${COMMON_LIB_DIR}/*.a ${COMMON_LIB_DIR}/*.so)
    if(COMMON_LIBS)
        message(STATUS "Found common libraries: ${COMMON_LIBS}")
        target_link_libraries(common_interface 
            INTERFACE 
                ${COMMON_LIBS}
        )
    endif()

    # ==================== 导入 mtlog 库 ====================
    # 检查 mtlog 库是否存在
    set(MTLOG_STATIC_LIB ${COMMON_LOCAL_LIB_DIR}/libmtlog.a)
    set(MTLOG_SHARED_LIB ${COMMON_LOCAL_LIB_DIR}/libmtlog.so)
    
    if(EXISTS ${MTLOG_STATIC_LIB})
        message(STATUS "Found mtlog static library: ${MTLOG_STATIC_LIB}")
        
        # 创建 IMPORTED 库
        add_library(mtlog_static STATIC IMPORTED)
        set_target_properties(mtlog_static PROPERTIES
            IMPORTED_LOCATION ${MTLOG_STATIC_LIB}
        )
        
        # 设置 mtlog 的头文件路径（假设在 mtlog/source 下）
        target_include_directories(mtlog_static 
            INTERFACE 
                ${PROJECT_ROOT}/20-mtcbb/mtlog/source
        )
        
        # 链接 mtlog 的依赖
        target_link_libraries(mtlog_static
            INTERFACE
                Boost::log
                Boost::log_setup
                Boost::system
                Boost::thread
                Boost::filesystem
        )
        
        # 创建别名
        add_library(mtlog ALIAS mtlog_static)
        
    elseif(EXISTS ${MTLOG_SHARED_LIB})
        message(STATUS "Found mtlog shared library: ${MTLOG_SHARED_LIB}")
        
        add_library(mtlog_shared SHARED IMPORTED)
        set_target_properties(mtlog_shared PROPERTIES
            IMPORTED_LOCATION ${MTLOG_SHARED_LIB}
        )
        
        target_include_directories(mtlog_shared 
            INTERFACE 
                ${PROJECT_ROOT}/20-mtcbb/mtlog/source
        )
        
        target_link_libraries(mtlog_shared
            INTERFACE
                Boost::log
                Boost::log_setup
                Boost::system
                Boost::thread
                Boost::filesystem
        )
        
        add_library(mtlog ALIAS mtlog_shared)
        
    else()
        message(FATAL_ERROR 
            "mtlog library not found!\n"
            "Please build mtlog first:\n"
            "  cd ${PROJECT_ROOT}/20-mtcbb/mtlog && mkdir build && cd build\n"
            "  cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} .. && make"
        )
    endif()
endif()

# ==================== demo 可执行文件 ====================
# 收集源文件
file(GLOB_RECURSE DEMO_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

# 创建可执行文件
add_executable(demo ${DEMO_SOURCES})

target_include_directories(demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接依赖
target_link_libraries(demo 
    PRIVATE 
        common_interface
        mtlog
)

# 编译选项
target_compile_options(demo PRIVATE
    -Wall
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

# ==================== 安装/拷贝（可选） ====================
if(DEFINED COMMON_BIN_OUTPUT_DIR)
    add_custom_command(TARGET demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
            $<TARGET_FILE:demo>
            ${COMMON_BIN_OUTPUT_DIR}/demo
        COMMENT "Copying demo to ${COMMON_BIN_OUTPUT_DIR}"
    )
endif()