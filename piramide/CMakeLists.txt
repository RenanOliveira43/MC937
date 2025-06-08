cmake_minimum_required(VERSION 3.5)

project(flyinGL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW config para Linux
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_BUILD_X11 ON)

# Adiciona GLEW, LFW e GLM
add_subdirectory(external/glew-2.2.0/build/cmake)
add_subdirectory(external/glfw-3.4)
add_subdirectory(external/glm)

find_package(OpenGL REQUIRED)

add_executable(flyinGL main.cpp)

# Adiciona os includes do GLEW
target_include_directories(flyinGL PRIVATE external/glew-2.2.0/include)

# Linka tudo
target_link_libraries(flyinGL PRIVATE glm OpenGL::GL glfw glew_s)
