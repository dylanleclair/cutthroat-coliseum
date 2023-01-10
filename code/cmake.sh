cmake -B ..\build\ -S . "-DCMAKE_TOOLCHAIN_FILE=[vcpkg base]/scripts/buildsystems/vcpkg.cmake"
cmake --build ..\build\