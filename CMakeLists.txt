cmake_minimum_required(VERSION 3.16)
project(Renderer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(my_app
    src/main.cpp
    src/shader.cpp
    src/stb_image.cpp
    external/src/glad.c
)

target_include_directories(my_app PRIVATE
    external/include
    /opt/homebrew/include
)

target_link_directories(my_app PRIVATE /opt/homebrew/lib)

target_link_libraries(my_app PRIVATE glfw3
    "-framework Cocoa"
    "-framework OpenGL"
    "-framework IOKit"
    "-framework CoreVideo"
)
