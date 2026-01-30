# Findmtlog.cmake
# 用于查找 mtlog C++ Web 框架
#
# 使用方法:
#   find_package(mtlog REQUIRED)
#   target_link_libraries(your_target PRIVATE mtlog::mtlog)
#
# 定义的变量:
#   mtlog_FOUND        - 如果找到 mtlog 则为 TRUE
#   mtlog_INCLUDE_DIRS - mtlog 头文件目录
#   mtlog_LIBRARIES    - mtlog 库文件
#   mtlog_VERSION      - mtlog 版本号
#
# 定义的导入目标:
#   mtlog::mtlog      - 主库目标

# 首先尝试使用 mtlog 的 Config 模式（如果 mtlog 已经安装并提供了 mtlogConfig.cmake）
find_package(mtlog ${mtlog_FIND_VERSION} CONFIG QUIET)

if(mtlog_FOUND)
    # 如果找到了 Config 文件，直接返回
    return()
endif()

# 如果没有找到 Config 文件，使用 Module 模式手动查找

# 查找头文件
find_path(mtlog_INCLUDE_DIR
    NAMES mtlog.hpp
    PATHS
        ${DROGON_ROOT}
        $ENV{DROGON_ROOT}
        /usr/local
        /usr
        /opt/local
        /opt
        ${CMAKE_PREFIX_PATH}
    PATH_SUFFIXES
        include
    DOC "mtlog include directory"
)

# 查找库文件
find_library(mtlog_LIBRARY
    NAMES mtlog libmtlog
    PATHS
        ${DROGON_ROOT}
        $ENV{DROGON_ROOT}
        /usr/local
        /usr
        /opt/local
        /opt
        ${CMAKE_PREFIX_PATH}
    PATH_SUFFIXES
        lib
        lib64
        lib/x86_64-linux-gnu
    DOC "mtlog library"
)

# 尝试获取版本信息
if(mtlog_INCLUDE_DIR)
    if(EXISTS "${mtlog_INCLUDE_DIR}/mtlog/version.h")
        file(READ "${mtlog_INCLUDE_DIR}/mtlog/version.h" _mtlog_version_header)
        
        string(REGEX MATCH "#define[ \t]+DROGON_VERSION_MAJOR[ \t]+([0-9]+)" 
               _mtlog_major_version_match "${_mtlog_version_header}")
        set(mtlog_VERSION_MAJOR "${CMAKE_MATCH_1}")
        
        string(REGEX MATCH "#define[ \t]+DROGON_VERSION_MINOR[ \t]+([0-9]+)"
               _mtlog_minor_version_match "${_mtlog_version_header}")
        set(mtlog_VERSION_MINOR "${CMAKE_MATCH_1}")
        
        string(REGEX MATCH "#define[ \t]+DROGON_VERSION_PATCH[ \t]+([0-9]+)"
               _mtlog_patch_version_match "${_mtlog_version_header}")
        set(mtlog_VERSION_PATCH "${CMAKE_MATCH_1}")
        
        set(mtlog_VERSION "${mtlog_VERSION_MAJOR}.${mtlog_VERSION_MINOR}.${mtlog_VERSION_PATCH}")
        
        unset(_mtlog_version_header)
        unset(_mtlog_major_version_match)
        unset(_mtlog_minor_version_match)
        unset(_mtlog_patch_version_match)
    endif()
endif()

# 使用标准的 FindPackageHandleStandardArgs 处理结果
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mtlog
    REQUIRED_VARS
        mtlog_LIBRARY
        mtlog_INCLUDE_DIR
    VERSION_VAR
        mtlog_VERSION
    FAIL_MESSAGE
        "Could not find mtlog library. Try setting DROGON_ROOT to the mtlog installation directory."
)

# 设置输出变量
if(mtlog_FOUND)
    set(mtlog_LIBRARIES ${mtlog_LIBRARY})
    set(mtlog_INCLUDE_DIRS ${mtlog_INCLUDE_DIR})
    
    # 标记为高级变量（不在 CMake GUI 中显示）
    mark_as_advanced(
        mtlog_INCLUDE_DIR
        mtlog_LIBRARY
    )
    
    # 创建导入目标
    if(NOT TARGET mtlog::shared)
        add_library(mtlog::shared UNKNOWN IMPORTED)
        set_target_properties(mtlog::shared PROPERTIES
            IMPORTED_LOCATION "${mtlog_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${mtlog_INCLUDE_DIR}"
        )
        
        # mtlog 依赖这些库，添加到接口链接库
        find_package(Boost REQUIRED)
    endif()
endif()
