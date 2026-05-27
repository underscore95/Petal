
add_executable(Game
        Source/Main.cpp
        Source/GameCamera.cpp
        Source/GameCamera.h
)

target_link_libraries(Game PRIVATE Petal)