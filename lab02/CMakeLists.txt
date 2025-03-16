cmake_minimum_required(VERSION 3.5)

project(Lab2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(Lab2 main.cpp)

target_link_libraries(Lab2 PRIVATE GLEW::GLEW glfw OpenGL::GL)
