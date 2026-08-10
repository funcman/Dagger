# Engine: pixel / color / line drawing library.
# Inline assembly was moved to standalone files under asm/ (64-bit MSVC
# has no __asm support):
#   MSVC        -> asm/Engine64_msvc.asm (ml64, MS ABI)
#   GCC / Clang -> asm/Engine64_gnu.S  (GAS, System V ABI; MS ABI branch under MinGW)
if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Engine assembly routines are currently available for 64-bit targets only")
endif()

# Normalize the host of x86-64 / ARM64 spellings CMake may report.
set(DAGGER_X64_PROCESSORS x86_64 AMD64 x64)
set(DAGGER_ARM64_PROCESSORS arm64 aarch64 ARM64)
if (CMAKE_SYSTEM_PROCESSOR IN_LIST DAGGER_X64_PROCESSORS)
    set(DAGGER_IS_X64 TRUE)
else()
    set(DAGGER_IS_X64 FALSE)
endif()
if (CMAKE_SYSTEM_PROCESSOR IN_LIST DAGGER_ARM64_PROCESSORS)
    set(DAGGER_IS_ARM64 TRUE)
else()
    set(DAGGER_IS_ARM64 FALSE)
endif()

if (MSVC AND DAGGER_IS_X64)
    enable_language(ASM_MASM)
    set(ENGINE_ASM_SOURCES asm/Engine64_msvc.asm)
elseif(DAGGER_IS_X64)
    enable_language(ASM)
    set(ENGINE_ASM_SOURCES asm/Engine64_gnu.S)
elseif(DAGGER_IS_ARM64)
    enable_language(ASM)
    set(ENGINE_ASM_SOURCES asm/Engine64_arm64.S)
else()
    message(STATUS "Engine: architecture ${CMAKE_SYSTEM_PROCESSOR} is not x86-64 or arm64; using C++ fallback for asm routines")
    set(ENGINE_ASM_SOURCES asm/Engine64_fallback.cpp)
endif()

add_library(DEngine SHARED
    Canvas.cpp
    Color.cpp
    DrawLine.cpp
    DrawPixel.cpp
    Bitmap.cpp
    ${ENGINE_ASM_SOURCES}
)

# Auto-export all symbols on Windows; no need for __declspec(dllexport)
set_target_properties(DEngine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(DEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DEngine PUBLIC DBase)
