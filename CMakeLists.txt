cmake_minimum_required(VERSION 3.16)
project(Renderer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(my_app
    src/main.cpp
    utils/shader.cpp
    utils/stb_image.cpp
    utils/object.cpp
    utils/obj_loader.cpp
    utils/terrain.cpp
    utils/perlin_noise.cpp
    utils/texture.cpp
    utils/obj_placer.cpp
    external/src/glad.c
)

target_include_directories(my_app PRIVATE
    ${CMAKE_SOURCE_DIR}/external/include
)

# Make asset paths stable across OS + build dirs
target_compile_definitions(my_app PRIVATE
    PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

# ---- OpenGL ----
find_package(OpenGL REQUIRED)

# ---- GLFW ----
# Prefer GLFW's CMake package when available
find_package(glfw3 QUIET)

if (glfw3_FOUND)
    target_link_libraries(my_app PRIVATE OpenGL::GL glfw)
else()
    # Fallback: use pkg-config (common on Linux distros)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED glfw3)

    target_include_directories(my_app PRIVATE ${GLFW_INCLUDE_DIRS})
    target_link_libraries(my_app PRIVATE ${GLFW_LIBRARIES} OpenGL::GL)

    # Some setups need this for link flags from pkg-config
    target_link_options(my_app PRIVATE ${GLFW_LDFLAGS})
endif()

# ---- macOS frameworks (GLFW often needs these) ----
if (APPLE)
    target_link_libraries(my_app PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()
