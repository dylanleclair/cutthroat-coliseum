# cpsc585

## getting started

to get started on contributing to this project, you'll need to install `vcpkg`, a c++ package manager. you will likely also want to config your editor of choice to work with CMake, or you can use CMake at the command line (commands are in )

1. install `CMake` (not required if using Visual Studio)
2. from the root of the project, run `./init.sh` on mac/linux, or `.\init.ps1` on windows - this will install vcpkg and the project dependencies, then build the project. In fact, if you want to
build the project entirely using this script it'll work (I think)!
3. setup your IDE to run with the project. (if not using IDE, I'd recommend just using the commands from the `init` scripts manually)

- see the IDE integration section here:
  - https://vcpkg.io/en/docs/users/buildsystems/cmake-integration.html

## project organization

- the project is divided into three chunks right now.
  - lib - systems code / code that supports the game but could easily be reused in other games.
  - client - the actual game / game logic
  - tst - for unit tests
