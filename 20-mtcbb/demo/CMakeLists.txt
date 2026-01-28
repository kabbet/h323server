cmake_minimum_required(VERSION 3.5)
project(demo)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 构建选项
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_STATIC_LIBS "Build static libraries" OFF)
option(BUILD_EXECUTABLE "Build executable demo" ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
# 设置 Boost 路径 - 在 find_package 之前
set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/../../10-common")
set(BOOST_INCLUDEDIR "${CMAKE_SOURCE_DIR}/../../10-common/include")
set(BOOST_LIBRARYDIR "${CMAKE_SOURCE_DIR}/../../10-common/lib/releaselib/linux64")
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_NO_BOOST_CMAKE ON)
set(Boost_DEBUG OFF)  # 临时启用调试信息，看看 CMake 在哪里找 Boost
set(Boost_NO_WARN_NEW_VERSIONS ON)
include_directories(${CMAKE_SOURCE_DIR}/../../10-common/include)
link_directories(
    ${CMAKE_SOURCE_DIR}/../../10-common/lib/locallib/linux64/Debug
    ${CMAKE_SOURCE_DIR}/../../10-common/lib/releaselib/linux64
)
find_package(Boost REQUIRED COMPONENTS log log_setup thread filesystem)
if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../../10-common/lib/locallib/linux64/${CMAKE_BUILD_TYPE}")
set(mtlog_DIR "${CMAKE_SOURCE_DIR}/../../10-common/lib/locallib/linux64/${CMAKE_BUILD_TYPE}/cmake/mtlog")
find_package(mtlog REQUIRED)
# 源文件列表
set(${CMAKE_PROJECT_NAME}_SOURCES
    source/main.cpp
)
# 可执行文件
if(BUILD_EXECUTABLE)
    add_executable(${CMAKE_PROJECT_NAME} ${${CMAKE_PROJECT_NAME}_SOURCES})
    target_compile_definitions(${CMAKE_PROJECT_NAME}
        PUBLIC
            BOOST_ALL_DYN_LINK
    )
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
         mtlog::shared
    )
    install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION .)
endif()
