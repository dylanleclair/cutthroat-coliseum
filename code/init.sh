git submodule update --init --recursive
../vcpkg/bootstrap-vcpkg.sh
cmake -B ../build/ -S .
cmake --build ../build/

# example command to run built project from command line:
# ../build/client/Debug/cpsc585_client.exe
