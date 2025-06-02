cmake_minimum_required(VERSION 3.5)

project(piramide LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e
)
FetchContent_MakeAvailable(glm)

# GLFW config para Linux
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_BUILD_X11 ON)

# Adiciona GLEW e GLFW
add_subdirectory(external/glew-2.2.0/build/cmake)
add_subdirectory(external/glfw-3.4)

find_package(OpenGL REQUIRED)

add_executable(piramide main.cpp)

# Adiciona os includes do GLEW
target_include_directories(piramide PRIVATE external/glew-2.2.0/include)

# Linka tudo
target_link_libraries(piramide PRIVATE glm OpenGL::GL glfw glew_s)
