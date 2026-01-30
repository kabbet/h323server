# FindTrantor.cmake
# 用于查找 Trantor C++ Web 框架
#
# 使用方法:
#   find_package(Trantor REQUIRED)
#   target_link_libraries(your_target PRIVATE Trantor::Trantor)
#
# 定义的变量:
#   Trantor_FOUND        - 如果找到 Trantor 则为 TRUE
#   Trantor_INCLUDE_DIRS - Trantor 头文件目录
#   Trantor_LIBRARIES    - Trantor 库文件
#   Trantor_VERSION      - Trantor 版本号
#
# 定义的导入目标:
#   Trantor::Trantor      - 主库目标

# 首先尝试使用 Trantor 的 Config 模式（如果 Trantor 已经安装并提供了 TrantorConfig.cmake）
find_package(Trantor ${Trantor_FIND_VERSION} CONFIG QUIET)

if(Trantor_FOUND)
    # 如果找到了 Config 文件，直接返回
    return()
endif()

# 如果没有找到 Config 文件，使用 Module 模式手动查找

# 查找头文件
find_path(Trantor_INCLUDE_DIR
    NAMES trantor/exports.h
    PATHS
        ${TRANTOR_ROOT}
        $ENV{TRANTOR_ROOT}
        /usr/local
        /usr
        /opt/local
        /opt
        ${CMAKE_PREFIX_PATH}
    PATH_SUFFIXES
        include
    DOC "Trantor include directory"
)

# 查找库文件
find_library(Trantor_LIBRARY
    NAMES trantor libtrantor
    PATHS
        ${TRANTOR_ROOT}
        $ENV{TRANTOR_ROOT}
        /usr/local
        /usr
        /opt/local
        /opt
        ${CMAKE_PREFIX_PATH}
    PATH_SUFFIXES
        lib
        lib64
        lib/x86_64-linux-gnu
    DOC "Trantor library"
)

# 尝试获取版本信息
if(Trantor_INCLUDE_DIR)
    if(EXISTS "${Trantor_INCLUDE_DIR}/trantor/version.h")
        file(READ "${Trantor_INCLUDE_DIR}/trantor/version.h" _trantor_version_header)
        
        string(REGEX MATCH "#define[ \t]+TRANTOR_VERSION_MAJOR[ \t]+([0-9]+)" 
               _trantor_major_version_match "${_trantor_version_header}")
        set(Trantor_VERSION_MAJOR "${CMAKE_MATCH_1}")
        
        string(REGEX MATCH "#define[ \t]+TRANTOR_VERSION_MINOR[ \t]+([0-9]+)"
               _trantor_minor_version_match "${_trantor_version_header}")
        set(Trantor_VERSION_MINOR "${CMAKE_MATCH_1}")
        
        string(REGEX MATCH "#define[ \t]+TRANTOR_VERSION_PATCH[ \t]+([0-9]+)"
               _trantor_patch_version_match "${_trantor_version_header}")
        set(Trantor_VERSION_PATCH "${CMAKE_MATCH_1}")
        
        set(Trantor_VERSION "${Trantor_VERSION_MAJOR}.${Trantor_VERSION_MINOR}.${Trantor_VERSION_PATCH}")
        
        unset(_trantor_version_header)
        unset(_trantor_major_version_match)
        unset(_trantor_minor_version_match)
        unset(_trantor_patch_version_match)
    endif()
endif()

# 使用标准的 FindPackageHandleStandardArgs 处理结果
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Trantor
    REQUIRED_VARS
        Trantor_LIBRARY
        Trantor_INCLUDE_DIR
    VERSION_VAR
        Trantor_VERSION
    FAIL_MESSAGE
        "Could not find Trantor library. Try setting TRANTOR_ROOT to the Trantor installation directory."
)

# 设置输出变量
if(Trantor_FOUND)
    set(Trantor_LIBRARIES ${Trantor_LIBRARY})
    set(Trantor_INCLUDE_DIRS ${Trantor_INCLUDE_DIR})
    
    # 标记为高级变量（不在 CMake GUI 中显示）
    mark_as_advanced(
        Trantor_INCLUDE_DIR
        Trantor_LIBRARY
    )
    
    # 创建导入目标
    if(NOT TARGET Trantor::Trantor)
        add_library(Trantor::Trantor UNKNOWN IMPORTED)
        set_target_properties(Trantor::Trantor PROPERTIES
            IMPORTED_LOCATION "${Trantor_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Trantor_INCLUDE_DIR}"
        )
        
        # Trantor 依赖这些库，添加到接口链接库
        find_package(Threads REQUIRED)
        set_property(TARGET Trantor::Trantor APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES Threads::Threads
        )
        
        # 如果是共享库，可能还需要其他依赖
        if(UNIX AND NOT APPLE)
            set_property(TARGET Trantor::Trantor APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES uuid dl
            )
        endif()
    endif()
endif()
