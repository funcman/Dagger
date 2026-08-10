add_library(DBase SHARED
    Debug.cpp
    Memory.cpp
    MemPool.cpp
    Node.cpp
    List.cpp
)

# Auto-export all symbols on Windows; no need for __declspec(dllexport)
set_target_properties(DBase PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(DBase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
