#!/bin/bash

# Build script for Open Wanzer with local raylib

echo "Building Open Wanzer..."

# Check if raylib exists locally
if [ ! -f "lib/libraylib.a" ] && [ ! -f "lib/libraylib.so" ]; then
    echo "Error: raylib not found in ./lib/"
    echo "Please ensure raylib is installed in the lib/ directory"
    exit 1
fi

# Check if headers exist
if [ ! -f "include/raylib.h" ]; then
    echo "Error: raylib.h not found in ./include/"
    exit 1
fi

# Compile with local raylib
g++ openwanzer.cpp -o openwanzer \
    -I./include \
    -L./lib \
    -lraylib \
    -lm \
    -lpthread \
    -ldl \
    -lrt \
    -lX11 \
    -Wl,-rpath=./lib \
    -std=c++17 \
    -O2 \
    -Wall

if [ $? -eq 0 ]; then
    echo "Build successful! Run with: ./openwanzer"
else
    echo "Build failed!"
    exit 1
fi
