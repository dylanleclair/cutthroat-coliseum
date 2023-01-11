# cpsc585

## getting started

1. install `CMake` if not on windows, or "Desktop Development with C++" in the visual studio installer in windows (it provides cmake)
2. from the root of the project, run `./init.sh` on mac/linux, or `.\init.ps1` on windows - this will install and bootstrap vcpkg on windows and on mac/linux it builds the project as well
3. setup your IDE to run with the project. (if not using IDE, I'd recommend just using the commands from the `init` scripts manually)
- Install "Desktop Development with C++" in the visual studio installer (for cmake) if you haven't already
- It will ask which cmake thing to use, use <project_root>/code/CMakeLists.txt
- Everything should build for you wooooo lmk if there's an issue

- see the IDE integration section here:
  - https://vcpkg.io/en/docs/users/buildsystems/cmake-integration.html

## project organization

- the project is divided into three chunks right now.
  - lib - systems code / code that supports the game but could easily be reused in other games.
  - client - the actual game / game logic
  - tst - for unit tests
