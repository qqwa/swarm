#!/bin/bash

# exit when any command fails
set -e

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make `nproc`
cd ..
./build/swarm cfg/default.toml