#!/bin/bash

# Function to perform the default action
default_action() {
  cd lunix-sys
  mkdir build
  cd build
  cmake ..
  make -j$(nproc)
  cd ../../lunix-bl/build
  cmake ..
  make -j$(nproc)
  ./post-build.sh
  echo "Done. To run check for build errors then run ./Run.sh"
}

# Function to perform the rebuild action
rebuild_action() {
  cd lunix-sys/build
  make -j$(nproc)
  cd ../../lunix-bl/build
  make -j$(nproc)
}

# Check if any arguments are passed
if [ "$#" -eq 0 ]; then
  # No arguments passed
  default_action
elif [ "$1" == "--rebuild" ]; then
  # --rebuild argument passed
  rebuild_action
else
  echo "Unknown argument: $1"
  echo "Usage: $0 [--rebuild]"
fi
