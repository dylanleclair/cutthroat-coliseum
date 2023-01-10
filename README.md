# cpsc585

## getting started

to get started on contributing to this project, you'll need to install `vcpkg`, a c++ package manager. you will likely also want to config your editor of choice to work with CMake, or you can use CMake at the command line (commands are in )

1. install `CMake` (not required if using Visual Studio)
2. install `vcpkg` by following the instructions here: https://vcpkg.io/en/getting-started.html
3. set the CMAKE_TOOLCHAIN_FILE location to your vcpkg toolchain file:

- if you're using the scripts I've provided:
  - open up the `code/init.ps1` or `code/init.sh` (based on architecture)
  - modify the first line, changing `-DCMAKE_TOOLCHAIN_FILE=[vcpkg base]/scripts/buildsystems/vcpkg.cmake` by subbing in the folder where you installed vcpkg into `[vcpkg base]`
    - for example, mine ended up as: `-DCMAKE_TOOLCHAIN_FILE=R:/newrepos/vcpkg/scripts/buildsystems/vcpkg.cmake"`
  - run the script:
    - `cd code`
    - `./init.ps1`

4. setup your IDE to run with the project. (if not using IDE, I'd recommend just using the commands from the `init` scripts manually)

- see the IDE integration section here:
  - https://vcpkg.io/en/docs/users/buildsystems/cmake-integration.html

## project organization

- the project is divided into three chunks right now.
  - lib - systems code / code that supports the game but could easily be reused in other games.
  - client - the actual game / game logic
  - tst - for unit tests
