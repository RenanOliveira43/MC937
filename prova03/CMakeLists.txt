cmake_minimum_required(VERSION 3.5)

project(prova03 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuração do GLFW para Linux
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_BUILD_X11 ON)

# GLM via FetchContent
include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e
)

FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)

# Fontes
add_executable(prova03 main.cpp)
add_library(aabb aabb.cpp)

# Inclui diretórios de cabeçalho


# Vincula bibliotecas
target_link_libraries(aabb PUBLIC glm::glm)
target_link_libraries(prova03 PRIVATE glm::glm OpenGL::GL glfw GLEW::GLEW aabb)
