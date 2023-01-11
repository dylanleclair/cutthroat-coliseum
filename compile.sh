#!/usr/bin/env sh

cmake -B build/ -S code/

# HACK(beau): compiling all targets breaks for some reason on my machine - beau
cmake --build build/ --target client
