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


# 如果单独编译此子项目，需要设置路径
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # 单独编译模式
    get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
    set(BOOST_ROOT "${PROJECT_ROOT}/10-common")
    set(BOOST_INCLUDEDIR "${PROJECT_ROOT}/10-common/include")
    set(BOOST_LIBRARYDIR "${PROJECT_ROOT}/10-common/lib/releaselib/linux64")
    set(Boost_NO_SYSTEM_PATHS ON)
    set(Boost_NO_BOOST_CMAKE ON)

    # mtlog
    set(DROGON_ROOT "${PROJECT_ROOT}/10-common")
    set(DROGON_INCLUDEDIR "${PROJECT_ROOT}/10-common/include")
    set(DROGON_LIBRARYDIR "${PROJECT_ROOT}/10-common/lib/releaselib/linux64")



    include_directories(${PROJECT_ROOT}/10-common/include)
    link_directories(
        ${PROJECT_ROOT}/10-common/lib/locallib/linux64/Debug
        ${PROJECT_ROOT}/10-common/lib/releaselib/linux64
    )

    list(APPEND CMAKE_PREFIX_PATH "${PROJECT_ROOT}/10-common/lib/locallib/linux64/${CMAKE_BUILD_TYPE}")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    set(mtlog_DIR "${PROJECT_ROOT}/10-common/lib/locallib/linux64/${CMAKE_BUILD_TYPE}/cmake/mtlog")
endif()

set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(Boost REQUIRED COMPONENTS log log_setup thread filesystem)
if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    find_package(mtlog MODULE REQUIRED)
endif()

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
