cmake_minimum_required(VERSION 3.5)
project(demo)

# 构建选项
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_STATIC_LIBS "Build static libraries" OFF)
option(BUILD_EXECUTABLE "Build executable demo" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include_directories(${CMAKE_SOURCE_DIR}/../../10-common/include)
link_directories(${CMAKE_SOURCE_DIR}/../../10-common/lib/locallib/linux64/Debug)

# 源文件列表
set(${CMAKE_PROJECT_NAME}_SOURCES
    source/main.cpp
)

# 可执行文件
if(BUILD_EXECUTABLE)
    add_executable(${CMAKE_PROJECT_NAME} ${${CMAKE_PROJECT_NAME}_SOURCES})
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE mtlog)
    install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION .)
endif()
