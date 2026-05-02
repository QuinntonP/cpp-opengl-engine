cmake_minimum_required(VERSION 3.16)
project(Renderer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- OpenGL ----
find_package(OpenGL REQUIRED)

# ---- GLFW ----
find_package(glfw3 QUIET)

if (NOT glfw3_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED glfw3)
endif()

# ============================================================
# Engine library
# ============================================================

add_library(Engine
    Engine/src/shader.cpp
    Engine/src/stb_image.cpp
    Engine/src/object.cpp
    Engine/src/obj_loader.cpp
    Engine/src/terrain.cpp
    Engine/src/perlin_noise.cpp
    Engine/src/texture.cpp
    Engine/src/obj_placer.cpp
    external/src/glad.c
)

target_include_directories(Engine PUBLIC
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${CMAKE_SOURCE_DIR}/external/include
)

target_link_libraries(Engine PUBLIC
    OpenGL::GL
)

# Make asset paths stable across OS + build dirs
target_compile_definitions(Engine PUBLIC
    PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

# ============================================================
# Sandbox executable
# ============================================================

add_executable(Sandbox
    Sandbox/src/main.cpp
)

target_link_libraries(Sandbox PRIVATE
    Engine
)

if (glfw3_FOUND)
    target_link_libraries(Sandbox PRIVATE glfw)
else()
    target_include_directories(Sandbox PRIVATE ${GLFW_INCLUDE_DIRS})
    target_link_libraries(Sandbox PRIVATE ${GLFW_LIBRARIES})
    target_link_options(Sandbox PRIVATE ${GLFW_LDFLAGS})
endif()

# ============================================================
# Editor executable
# ============================================================

add_executable(Editor
    Editor/src/main.cpp
)

target_link_libraries(Editor PRIVATE
    Engine
)

if (glfw3_FOUND)
    target_link_libraries(Editor PRIVATE glfw)
else()
    target_include_directories(Editor PRIVATE ${GLFW_INCLUDE_DIRS})
    target_link_libraries(Editor PRIVATE ${GLFW_LIBRARIES})
    target_link_options(Editor PRIVATE ${GLFW_LDFLAGS})
endif()

# ============================================================
# macOS frameworks
# ============================================================

if (APPLE)
    target_link_libraries(Sandbox PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )

    target_link_libraries(Editor PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()