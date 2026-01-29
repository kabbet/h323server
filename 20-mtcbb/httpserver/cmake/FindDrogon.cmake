# FindDrogon.cmake
# 用于查找 Drogon C++ Web 框架
#
# 使用方法:
#   find_package(Drogon REQUIRED)
#   target_link_libraries(your_target PRIVATE Drogon::Drogon)
#
# 定义的变量:
#   Drogon_FOUND        - 如果找到 Drogon 则为 TRUE
#   Drogon_INCLUDE_DIRS - Drogon 头文件目录
#   Drogon_LIBRARIES    - Drogon 库文件
#   Drogon_VERSION      - Drogon 版本号
#
# 定义的导入目标:
#   Drogon::Drogon      - 主库目标

# 首先尝试使用 Drogon 的 Config 模式（如果 Drogon 已经安装并提供了 DrogonConfig.cmake）
find_package(Drogon ${Drogon_FIND_VERSION} CONFIG QUIET)

if(Drogon_FOUND)
    # 如果找到了 Config 文件，直接返回
    return()
endif()

# 如果没有找到 Config 文件，使用 Module 模式手动查找

# 查找头文件
find_path(Drogon_INCLUDE_DIR
    NAMES drogon/drogon.h
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
    DOC "Drogon include directory"
)

# 查找库文件
find_library(Drogon_LIBRARY
    NAMES drogon libdrogon
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
    DOC "Drogon library"
)

# 尝试获取版本信息
if(Drogon_INCLUDE_DIR)
    if(EXISTS "${Drogon_INCLUDE_DIR}/drogon/version.h")
        file(READ "${Drogon_INCLUDE_DIR}/drogon/version.h" _drogon_version_header)
        
        string(REGEX MATCH "#define[ \t]+DROGON_VERSION_MAJOR[ \t]+([0-9]+)" 
               _drogon_major_version_match "${_drogon_version_header}")
        set(Drogon_VERSION_MAJOR "${CMAKE_MATCH_1}")
        
        string(REGEX MATCH "#define[ \t]+DROGON_VERSION_MINOR[ \t]+([0-9]+)"
               _drogon_minor_version_match "${_drogon_version_header}")
        set(Drogon_VERSION_MINOR "${CMAKE_MATCH_1}")
        
        string(REGEX MATCH "#define[ \t]+DROGON_VERSION_PATCH[ \t]+([0-9]+)"
               _drogon_patch_version_match "${_drogon_version_header}")
        set(Drogon_VERSION_PATCH "${CMAKE_MATCH_1}")
        
        set(Drogon_VERSION "${Drogon_VERSION_MAJOR}.${Drogon_VERSION_MINOR}.${Drogon_VERSION_PATCH}")
        
        unset(_drogon_version_header)
        unset(_drogon_major_version_match)
        unset(_drogon_minor_version_match)
        unset(_drogon_patch_version_match)
    endif()
endif()

# 使用标准的 FindPackageHandleStandardArgs 处理结果
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Drogon
    REQUIRED_VARS
        Drogon_LIBRARY
        Drogon_INCLUDE_DIR
    VERSION_VAR
        Drogon_VERSION
    FAIL_MESSAGE
        "Could not find Drogon library. Try setting DROGON_ROOT to the Drogon installation directory."
)

# 设置输出变量
if(Drogon_FOUND)
    set(Drogon_LIBRARIES ${Drogon_LIBRARY})
    set(Drogon_INCLUDE_DIRS ${Drogon_INCLUDE_DIR})
    
    # 标记为高级变量（不在 CMake GUI 中显示）
    mark_as_advanced(
        Drogon_INCLUDE_DIR
        Drogon_LIBRARY
    )
    
    # 创建导入目标
    if(NOT TARGET Drogon::Drogon)
        add_library(Drogon::Drogon UNKNOWN IMPORTED)
        set_target_properties(Drogon::Drogon PROPERTIES
            IMPORTED_LOCATION "${Drogon_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Drogon_INCLUDE_DIR}"
        )
        
        # Drogon 依赖这些库，添加到接口链接库
        find_package(Threads REQUIRED)
        set_property(TARGET Drogon::Drogon APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES Threads::Threads
        )
        
        # 如果是共享库，可能还需要其他依赖
        if(UNIX AND NOT APPLE)
            set_property(TARGET Drogon::Drogon APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES uuid dl
            )
        endif()
    endif()
endif()
