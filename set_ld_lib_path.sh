#!/bin/bash

# Go back up to the Symbi-OS directory
cd ..

# Get the current path
CUR_DIR=$(pwd)

# Exporting the environment variable
LD_LIBRARY_PATH="'$CUR_DIR/Symlib/dynam_build/'"

# Testing the env variable
echo "Run the following command..."
echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo ""
