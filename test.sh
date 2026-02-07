#!/bin/bash

WORK_DIR=$(pwd)
BUILD_DIR=${WORK_DIR}/build

# Build the project
./build.sh

# Run tests
cd ${BUILD_DIR}
ctest --output-on-failure -V