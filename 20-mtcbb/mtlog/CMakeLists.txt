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
set(${CMAKE_PROJECT_NAME}_SOURCES
    source/mtlog.cpp
)

# 动态库
if(BUILD_SHARED_LIBS)
    add_library(${CMAKE_PROJECT_NAME}_shared SHARED ${${CMAKE_PROJECT_NAME}_SOURCES})
    set_target_properties(${CMAKE_PROJECT_NAME}_shared PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})
    
    target_compile_definitions(${CMAKE_PROJECT_NAME}_shared
        PUBLIC
            BOOST_ALL_DYN_LINK
    )
    
    target_link_libraries(${CMAKE_PROJECT_NAME}_shared
        PRIVATE 
            -Wl,--no-as-needed
            boost_log_setup
            boost_log
            boost_thread
            boost_filesystem
            -Wl,--as-needed
    )
    
    install(TARGETS ${CMAKE_PROJECT_NAME}_shared LIBRARY DESTINATION .)
endif()

# 静态库
if(BUILD_STATIC_LIBS)
    add_library(${CMAKE_PROJECT_NAME}_static STATIC ${CMAKE_PROJECT_NAME}_SOURCES)
    set_target_properties(${CMAKE_PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})
    
    target_link_libraries(${CMAKE_PROJECT_NAME}_static
        PRIVATE 
            boost_log_setup
            boost_log
            boost_thread
            boost_filesystem
            pthread
    )
    
    install(TARGETS ${CMAKE_PROJECT_NAME}_static ARCHIVE DESTINATION .)
endif()
