#!/bin/bash

cmake -B build/ -S code/
cmake --build build/ --target client

pushd build/client || exit
./cpsc585_client
popd || exit
