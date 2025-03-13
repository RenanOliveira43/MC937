cmake_minimum_required(VERSION 3.5)

project(Lab1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})

add_executable(Lab2 main.cpp)

target_link_libraries(Lab2 ${OpenCV_LIBS} ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} ${OPENGL_gl_LIBRARY})