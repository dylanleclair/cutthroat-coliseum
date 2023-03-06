set(BINARY ${CMAKE_PROJECT_NAME}_client)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.hpp *.frag *.vert)

set(SOURCES ${SOURCES})

add_executable(${BINARY} ${SOURCES})
set_property(TARGET ${BINARY} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")

# PhysX does not have a find_package setup therefore we have to find it manually
# https://github.com/microsoft/vcpkg/issues/10233
include(SelectLibraryConfigurations)

# the dlls need be copied to the output directory!! configure_file those bad boys

set(PHYSX_LIB_PATH "${CMAKE_SOURCE_DIR}/thirdparty/physx/Release/")
set(PHYSX_LIB_DEBUG_PATH "${CMAKE_SOURCE_DIR}/thirdparty/physx/Debug/")
set(PHYSX_LIB_DIR "${CMAKE_SOURCE_DIR}/thirdparty/physx/")

message( ${PHYSX_LIB_DIR} )
message( ${PHYSX_LIB_PATH} )
message( ${PHYSX_LIB_DEBUG_PATH} )

if(MSVC)
 target_compile_options(${CMAKE_PROJECT_NAME}_client PRIVATE "/MP")
endif()

# target_link_directories(${BINARY} PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/physx/debug/")


find_library(PhysXPvdSDK_LIBRARY_RELEASE NAMES PhysXPvdSDK_64 PhysXPvdSDK_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_PATH} NO_DEFAULT_PATH REQUIRED)
find_library(PhysXPvdSDK_LIBRARY_DEBUG NAMES PhysXPvdSDK_64 PhysXPvdSDK_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_DEBUG_PATH} NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXPvdSDK)

find_library(PhysXExtensions_LIBRARY_RELEASE NAMES PhysXExtensions_64 PhysXExtensions_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_PATH} NO_DEFAULT_PATH REQUIRED)
find_library(PhysXExtensions_LIBRARY_DEBUG NAMES PhysXExtensions_64 PhysXExtensions_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_DEBUG_PATH} NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXExtensions)

find_library(PhysXFoundation_LIBRARY_RELEASE NAMES PhysXFoundation_64 PhysXFoundation_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_PATH} NO_DEFAULT_PATH REQUIRED)
find_library(PhysXFoundation_LIBRARY_DEBUG NAMES PhysXFoundation_64 PhysXFoundation_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_DEBUG_PATH} NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXFoundation)

find_library(PhysXCommon_LIBRARY_RELEASE NAMES PhysXCommon_64 PhysXCommon_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_PATH} NO_DEFAULT_PATH REQUIRED)
find_library(PhysXCommon_LIBRARY_DEBUG NAMES PhysXCommon_64 PhysXCommon_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_DEBUG_PATH} NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXCommon)

find_library(PhysXVehicle2_LIBRARY_RELEASE NAMES PhysXVehicle2_64 PhysXVehicle2_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_PATH} NO_DEFAULT_PATH REQUIRED)
find_library(PhysXVehicle2_LIBRARY_DEBUG NAMES PhysXVehicle2_64 PhysXVehicle2_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_DEBUG_PATH} NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXVehicle2)


find_library(PhysXCooking_LIBRARY_RELEASE NAMES PhysXCooking_64 PhysXCooking_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_PATH} NO_DEFAULT_PATH REQUIRED)
find_library(PhysXCooking_LIBRARY_DEBUG NAMES PhysXCooking_64 PhysXCooking_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_DEBUG_PATH} NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXCooking)


find_library(PhysX_LIBRARY_RELEASE NAMES PhysX_64 PhysX_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_PATH} NO_DEFAULT_PATH REQUIRED)
find_library(PhysX_LIBRARY_DEBUG NAMES PhysX_64 PhysX_static_64 PATH_SUFFIXES lib PATHS ${PHYSX_LIB_DEBUG_PATH} NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysX)


set(FMOD_LIB_PATH ${CMAKE_SOURCE_DIR}/thirdparty/fmod/lib/)

find_library(FMOD_LIB fmod_vc HINTS ${FMOD_LIB_PATH})
find_library(FMODL_LIB fmodL_vc HINTS ${FMOD_LIB_PATH})

if(WIN32 OR APPLE)
  target_link_libraries(${BINARY} PRIVATE ${CMAKE_PROJECT_NAME}_lib 
    
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    glm::glm
    GLEW::GLEW
    assimp::assimp
    vivid
    fmt
    tomlplusplus::tomlplusplus
    imgui::imgui
    ${FMOD_LIB}
    ${FMODL_LIB}
    
    ${PhysXPvdSDK_LIBRARY} ${PhysXExtensions_LIBRARY} ${PhysXFoundation_LIBRARY} ${PhysXCommon_LIBRARY} ${PhysXCooking_LIBRARY} ${PhysXVehicle2_LIBRARY}  ${PhysX_LIBRARY} 
  )

else()
  target_link_libraries(${BINARY} PRIVATE ${CMAKE_PROJECT_NAME}_lib 
    
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    glm::glm
    GLEW::GLEW
    assimp::assimp
    vivid
    fmt
    tomlplusplus::tomlplusplus
    imgui::imgui
    
     -Wl,--start-group ${PhysXPvdSDK_LIBRARY} ${PhysXExtensions_LIBRARY} ${PhysXFoundation_LIBRARY} ${PhysXCommon_LIBRARY} ${PhysXCooking_LIBRARY} ${PhysXVehicle2_LIBRARY}  ${PhysX_LIBRARY} -Wl,--end-group
  )
endif()

target_compile_definitions(${BINARY} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../assets/") # Set the asset path macro to the absolute path on the dev machine

# find some way to choose right dll directory

add_custom_command(TARGET ${BINARY} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PHYSX_LIB_DIR}/$<CONFIGURATION>/shared/"
        $<TARGET_FILE_DIR:${BINARY}>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets/"
        $<TARGET_FILE_DIR:${BINARY}>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${FMOD_LIB_PATH}/shared/"
        $<TARGET_FILE_DIR:${BINARY}>)