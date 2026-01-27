cmake_minimum_required(VERSION 3.5)
project(mtlog)

# 构建选项
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" OFF)
option(BUILD_EXECUTABLE "Build executable demo" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include_directories(${CMAKE_SOURCE_DIR}/../../10-common/include)
link_directories(${CMAKE_SOURCE_DIR}/../../10-common/lib/releaselib/linux64)

# 源文件列表
set(MTLOG_SOURCES
    source/mtlog.cpp
)

# 动态库
if(BUILD_SHARED_LIBS)
    add_library(mtlog_shared SHARED ${MTLOG_SOURCES})
    set_target_properties(mtlog_shared PROPERTIES OUTPUT_NAME mtlog)
    
    target_compile_definitions(mtlog_shared
        PUBLIC
            BOOST_ALL_DYN_LINK
    )
    
    # 使用 -Wl,--no-as-needed 防止链接器优化掉未直接使用的库
    target_link_libraries(mtlog_shared
        PRIVATE 
            -Wl,--no-as-needed
            boost_log_setup
            boost_log
            boost_thread
            -Wl,--as-needed
    )
    
    install(TARGETS mtlog_shared LIBRARY DESTINATION lib)
endif()

# 静态库
if(BUILD_STATIC_LIBS)
    add_library(mtlog_static STATIC ${MTLOG_SOURCES})
    set_target_properties(mtlog_static PROPERTIES OUTPUT_NAME mtlog)
    
    target_link_libraries(mtlog_static
        PRIVATE 
            boost_log_setup
            boost_log
            boost_thread
            pthread
    )
    
    install(TARGETS mtlog_static ARCHIVE DESTINATION lib)
endif()