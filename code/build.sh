#!/bin/bash

mkdir -p ./build
pushd ./build
gcc ../code/sdl_handmade.c -o "Handmade Hero" -g `sdl2-config --cflags --libs`
popd
