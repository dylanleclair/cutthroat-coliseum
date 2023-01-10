cmake -B ..\build\ -S . "-DCMAKE_TOOLCHAIN_FILE=[vcpkg base]/scripts/buildsystems/vcpkg.cmake"
cmake --build ..\build\

# example command to run built project from command line:
# ../build/client/Debug/cpsc585_client.exe