#!/usr/bin/env sh

# HACK(beau): shader paths are relative to executable's directory
cd build/client || exit
./cpsc585_client
