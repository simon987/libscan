#!/usr/bin/env bash

export CC=clang
export CXX=clang++

rm -rf CMakeFiles CMakeCache.txt
cmake -DCMAKE_TOOLCHAIN_FILE=/usr/share/vcpkg/scripts/buildsystems/vcpkg.cmake . || exit
make -j 4
