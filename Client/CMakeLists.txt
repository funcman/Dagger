add_executable(Client
    Main.cpp
    Screen.cpp
)

target_link_libraries(Client PRIVATE DBase DEngine SDL3::SDL3)
