# ----------------------------------------------------
# 1. CMake 版本要求和项目定义
# ----------------------------------------------------
cmake_minimum_required(VERSION 3.10)
project(simple_client C)

# ----------------------------------------------------
# 2. 定义包含目录 (Include Directories)
# ----------------------------------------------------
# 使用相对路径，指向所有 .h 文件所在的目录
include_directories(
    # Core 头文件
    Core/Inc
    # Application 头文件
    Applications/Inc
    # Modules 头文件
    Modules/Inc

    Protocol/Inc
)

# ----------------------------------------------------
# 3. 收集源文件 (Source Files)
# ----------------------------------------------------
# 定义一个变量来保存所有需要编译的 .c 文件
set(PROJECT_SOURCES
    # Core 源文件
    Core/Src/main.c
    
    # Application 源文件
    Applications/Src/LEDControl_Task.c
    
    # Modules 源文件
    Modules/Src/udp_client.c

    Protocol/Src/udp_protocol.c
)

# ----------------------------------------------------
# 4. 创建可执行目标
# ----------------------------------------------------
# 使用所有的源文件创建名为 simple_client 的可执行文件
add_executable(simple_client ${PROJECT_SOURCES})