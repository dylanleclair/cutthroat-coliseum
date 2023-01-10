cmake -B ..\build\ -S . "-DCMAKE_TOOLCHAIN_FILE=R:/newrepos/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build ..\build\
../build/client/Debug/cpsc585_client.exe