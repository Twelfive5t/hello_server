cmake_minimum_required(VERSION 3.22.1)

# =================== 包含模块文件 ===================
# 添加 cmake 模块路径
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# 导入版本模块配置（需在 project() 前执行）
include(VersionConfig)

# =================== 项目基本信息 ===================
project(hello_server VERSION ${VERSION})

# 导入通用模块配置
include(CommonConfig)
include(ClangTools)

# =================== 子目录 ===================
add_subdirectory(common/logger)
add_subdirectory(common/telemetry)
add_subdirectory(services/proto messages)
add_subdirectory(services/server)
