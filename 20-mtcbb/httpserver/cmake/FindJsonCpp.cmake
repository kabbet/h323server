# FindJsonCpp.cmake
# 查找 JsonCpp 库
#
# 使用方法:
#   find_package(JsonCpp REQUIRED)
#
# 定义的变量:
#   JsonCpp_FOUND        - 如果找到 JsonCpp 则为 TRUE
#   JsonCpp_INCLUDE_DIRS - JsonCpp 的头文件目录
#   JsonCpp_LIBRARIES    - JsonCpp 的库文件
#   JsonCpp_VERSION      - JsonCpp 的版本号

# 查找头文件
find_path(JsonCpp_INCLUDE_DIR
    NAMES json/json.h
    PATHS
        /usr/include/jsoncpp
        /usr/local/include/jsoncpp
        /opt/local/include/jsoncpp
        ${CMAKE_PREFIX_PATH}/include/jsoncpp
    PATH_SUFFIXES jsoncpp
)

# 查找库文件
find_library(JsonCpp_LIBRARY
    NAMES jsoncpp libjsoncpp
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        ${CMAKE_PREFIX_PATH}/lib
    PATH_SUFFIXES
        x86_64-linux-gnu
        i386-linux-gnu
)

# 尝试从头文件中提取版本信息
if(JsonCpp_INCLUDE_DIR AND EXISTS "${JsonCpp_INCLUDE_DIR}/json/version.h")
    file(STRINGS "${JsonCpp_INCLUDE_DIR}/json/version.h" JsonCpp_VERSION_MAJOR_LINE REGEX "^#define[ \t]+JSONCPP_VERSION_MAJOR[ \t]+[0-9]+")
    file(STRINGS "${JsonCpp_INCLUDE_DIR}/json/version.h" JsonCpp_VERSION_MINOR_LINE REGEX "^#define[ \t]+JSONCPP_VERSION_MINOR[ \t]+[0-9]+")
    file(STRINGS "${JsonCpp_INCLUDE_DIR}/json/version.h" JsonCpp_VERSION_PATCH_LINE REGEX "^#define[ \t]+JSONCPP_VERSION_PATCH[ \t]+[0-9]+")
    
    string(REGEX REPLACE "^#define[ \t]+JSONCPP_VERSION_MAJOR[ \t]+([0-9]+)" "\\1" JsonCpp_VERSION_MAJOR "${JsonCpp_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+JSONCPP_VERSION_MINOR[ \t]+([0-9]+)" "\\1" JsonCpp_VERSION_MINOR "${JsonCpp_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+JSONCPP_VERSION_PATCH[ \t]+([0-9]+)" "\\1" JsonCpp_VERSION_PATCH "${JsonCpp_VERSION_PATCH_LINE}")
    
    set(JsonCpp_VERSION "${JsonCpp_VERSION_MAJOR}.${JsonCpp_VERSION_MINOR}.${JsonCpp_VERSION_PATCH}")
endif()

# 使用标准的 find_package_handle_standard_args 处理结果
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JsonCpp
    REQUIRED_VARS
        JsonCpp_LIBRARY
        JsonCpp_INCLUDE_DIR
    VERSION_VAR
        JsonCpp_VERSION
)

# 设置输出变量
if(JsonCpp_FOUND)
    set(JsonCpp_LIBRARIES ${JsonCpp_LIBRARY})
    set(JsonCpp_INCLUDE_DIRS ${JsonCpp_INCLUDE_DIR})
    
    # 创建 IMPORTED 目标（推荐的现代 CMake 方式）
    if(NOT TARGET JsonCpp::JsonCpp)
        add_library(JsonCpp::JsonCpp UNKNOWN IMPORTED)
        set_target_properties(JsonCpp::JsonCpp PROPERTIES
            IMPORTED_LOCATION "${JsonCpp_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${JsonCpp_INCLUDE_DIR}"
        )
    endif()
endif()

# 标记为高级变量（在 cmake-gui 中默认隐藏）
mark_as_advanced(
    JsonCpp_INCLUDE_DIR
    JsonCpp_LIBRARY
)
