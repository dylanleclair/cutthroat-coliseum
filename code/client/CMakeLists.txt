set(BINARY ${CMAKE_PROJECT_NAME}_client)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

set(SOURCES ${SOURCES})

add_executable(${BINARY} ${SOURCES})


# PhysX does not have a find_package setup therefore we have to find it manually
# https://github.com/microsoft/vcpkg/issues/10233
include(SelectLibraryConfigurations)

find_library(PhysXPvdSDK_LIBRARY_RELEASE NAMES PhysXPvdSDK_64 PhysXPvdSDK_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" NO_DEFAULT_PATH REQUIRED)
find_library(PhysXPvdSDK_LIBRARY_DEBUG NAMES PhysXPvdSDK_64 PhysXPvdSDK_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXPvdSDK)

find_library(PhysXExtensions_LIBRARY_RELEASE NAMES PhysXExtensions_64 PhysXExtensions_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" NO_DEFAULT_PATH REQUIRED)
find_library(PhysXExtensions_LIBRARY_DEBUG NAMES PhysXExtensions_64 PhysXExtensions_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXExtensions)

find_library(PhysXFoundation_LIBRARY_RELEASE NAMES PhysXFoundation_64 PhysXFoundation_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" NO_DEFAULT_PATH REQUIRED)
find_library(PhysXFoundation_LIBRARY_DEBUG NAMES PhysXFoundation_64 PhysXFoundation_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXFoundation)

find_library(PhysXCommon_LIBRARY_RELEASE NAMES PhysXCommon_64 PhysXCommon_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" NO_DEFAULT_PATH REQUIRED)
find_library(PhysXCommon_LIBRARY_DEBUG NAMES PhysXCommon_64 PhysXCommon_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysXCommon)

find_library(PhysX_LIBRARY_RELEASE NAMES PhysX_64 PhysX_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" NO_DEFAULT_PATH REQUIRED)
find_library(PhysX_LIBRARY_DEBUG NAMES PhysX_64 PhysX_static_64 PATH_SUFFIXES lib PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PhysX)

set(PHYSX_INCLUDES_LIBS )

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
    
    ${PhysXPvdSDK_LIBRARY} ${PhysXExtensions_LIBRARY} ${PhysXFoundation_LIBRARY} ${PhysXCommon_LIBRARY} ${PhysX_LIBRARY}
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
    
     -Wl,--start-group ${PhysXPvdSDK_LIBRARY} ${PhysXExtensions_LIBRARY} ${PhysXFoundation_LIBRARY} ${PhysXCommon_LIBRARY} ${PhysX_LIBRARY} -Wl,--end-group
  )
endif()

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
  
  ${PhysXPvdSDK_LIBRARY} ${PhysXExtensions_LIBRARY} ${PhysXFoundation_LIBRARY} ${PhysXCommon_LIBRARY} ${PhysX_LIBRARY}
)

target_compile_definitions(${BINARY} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../assets/") # Set the asset path macro to the absolute path on the dev machine

# Copy all the shaders and tell the build system to re-run CMAKE if one of them changes
file(GLOB files ../assets/shaders/*)
foreach(file ${files})
	get_filename_component(name ${file} NAME)
	configure_file(${file} shaders/${name})
endforeach()

# Copy all the serialized toml files and tell the build system to re-run CMAKE if one of them changes
file(GLOB files ../assets/configs/*)
foreach(file ${files})
	get_filename_component(name ${file} NAME)
	configure_file(${file} configs/${name})
endforeach()
