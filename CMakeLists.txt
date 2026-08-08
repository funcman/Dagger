cmake_minimum_required(VERSION 3.16)

project(Dagger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件统一按 UTF-8 编译，避免 MSVC 在 GBK 代码页下误解中文注释
if (MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
endif()

# 统一输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(Base)
add_subdirectory(Engine)

# SDL3 前端依赖（3rd/SDL3 由 scripts 初始化）
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/3rd/SDL3/CMakeLists.txt)
    message(FATAL_ERROR "3rd/SDL3 不存在，请先运行 Scripts/build.ps1 或 scripts/build.sh 初始化")
endif()
add_subdirectory(3rd/SDL3)

add_subdirectory(Client)
