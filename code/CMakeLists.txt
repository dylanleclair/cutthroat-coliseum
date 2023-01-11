cmake_minimum_required(VERSION 3.16)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")
project(cpsc585 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glew REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(lib)

enable_testing()

add_subdirectory(client)
add_subdirectory(lib)
add_subdirectory(tst)

# GTest settings
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#-------------------------------------------------------------------------
# other dependencies to fetch that were required in old cpsc453 skeleton!
include(FetchContent)
FetchContent_Declare(
  vivid
  GIT_REPOSITORY https://github.com/gurki/vivid.git
  GIT_TAG        0ced0226cdd32baa2921c14e8ed121c52ad17f30
)
FetchContent_MakeAvailable(vivid)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        cd4af11efc9c622896a3e4cb599fa28668ca3d05
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.2.0
)
FetchContent_MakeAvailable(tomlplusplus)
