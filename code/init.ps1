git submodule update --init --recursive # XXX: IDK if I can do this in powershell
..\vcpkg\bootstrap-vcpkg.bat
cmake -B ..\build\ -S .
cmake --build ..\build\

# example command to run built project from command line:
# ../build/client/Debug/cpsc585_client.exe
