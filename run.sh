#!/bin/bash

# Change directory to your project root (adjust this if needed)
make clean
# Run CMake
cmake .

# Build the project
make

# Run the executable
./Build/bin/restserver 0