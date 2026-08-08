# Engine: pixel / color / line drawing library.
# Inline assembly was moved to standalone files under asm/ (64-bit MSVC
# has no __asm support):
#   MSVC        -> asm/Engine64_msvc.asm (ml64, MS ABI)
#   GCC / Clang -> asm/Engine64_gnu.S  (GAS, System V ABI; MS ABI branch under MinGW)
if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Engine assembly routines are currently available for x86-64 only")
endif()

if (MSVC)
    enable_language(ASM_MASM)
    set(ENGINE_ASM_SOURCES asm/Engine64_msvc.asm)
else()
    enable_language(ASM)
    set(ENGINE_ASM_SOURCES asm/Engine64_gnu.S)
endif()

add_library(DEngine SHARED
    Canvas.cpp
    Color.cpp
    DrawLine.cpp
    DrawPixel.cpp
    ${ENGINE_ASM_SOURCES}
)

# Auto-export all symbols on Windows; no need for __declspec(dllexport)
set_target_properties(DEngine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(DEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DEngine PUBLIC DBase)
