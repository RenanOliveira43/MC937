cmake_minimum_required(VERSION 3.5)

project(Lab2 LANGUAGES CXX C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar o GLFW com FetchContent
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

# Definir as políticas para OpenGL
cmake_policy(SET CMP0072 NEW)  
set(OpenGL_GL_PREFERENCE GLVND)  

# Encontrar o pacote OpenGL
find_package(OpenGL REQUIRED)

# Incluir o diretório do GLAD
include_directories(${CMAKE_SOURCE_DIR}/glad)

# Adicionar o executável e incluir o arquivo glad.c
add_executable(Lab2 main.cpp glad/glad.c)

# Linkar as bibliotecas necessárias (GLFW e OpenGL)
target_link_libraries(Lab2 PRIVATE glfw OpenGL::GL)
