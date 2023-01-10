cmake_minimum_required(VERSION 3.16)
project(cpsc585 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(lib)

enable_testing()

add_subdirectory(client)
add_subdirectory(lib)
add_subdirectory(tst)

# GTest settings
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)



