cmake_minimum_required(VERSION 3.10)

# Project name
project(snake)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# Add GLAD (OpenGL loader)
add_library(glad glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

# Find GLFW (window/input library)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

# Find GLM (math library)
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
include_directories(${GLM_INCLUDE_DIR})

# Main program
add_executable(game main.cpp)

# Link everything
target_link_libraries(game PRIVATE glad ${GLFW_LIBRARIES})