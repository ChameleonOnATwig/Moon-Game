#!/bin/bash

mkdir -p ./build
pushd ./build
gcc ../src/sdl_handmade.c -o "Handmade Hero" -g `sdl2-config --cflags --libs`
popd
