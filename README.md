# cpsc585

## getting started (windows)

1. install "Desktop Development with C++" in the visual studio installer (it provides `cmake`)
1. open powershell and cd to the root directory of the project
1. run the command `Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process` (still in powershell in the root dir)
1. run the command `.\init.ps1` in powershell (still in powershell in the root dir)
1. setup your IDE to run with the project. (if not using IDE, I'd recommend just using the commands from the `init` scripts manually)
- Install "Desktop Development with C++" in the visual studio installer (for cmake) if you haven't already
- It will ask which cmake thing to use, use <project_root>/code/CMakeLists.txt
- Everything should build for you wooooo lmk if there's an issue
1. To run, use the green button dropdown to find `cpsc585-client.exe` or someth it'll work I think

- see the IDE integration section here:
  - https://vcpkg.io/en/docs/users/buildsystems/cmake-integration.html
  
NOTE: dylan i think you and i are fine without instructions

## project organization

- the project is divided into three chunks right now.
  - lib - systems code / code that supports the game but could easily be reused in other games.
  - client - the actual game / game logic
  - tst - for unit tests

## Hot-reload example

Version-controlled configs are stored in code/assets/configs. These are
then copied into build/client/configs, similar to the shaders.

DESERIALIZING:

1. Modify the config file in build/client/configs/CarPhysics.toml with
thing running
2. Press `t` - you should see the console print what was deserialized

SERIALIZING:

Press `s` to serialize your new config back into the version-controlled
config file. Note that this actually reads your config again cause idk
if the serializer should know abt the state of the program
