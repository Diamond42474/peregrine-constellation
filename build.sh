#!/bin/bash

WORK_DIR=$(pwd)
BUILD_DIR=${WORK_DIR}/build
DOCUMENT_DIR=${WORK_DIR}/docs

# Create build directory if it doesn't exist
if [ ! -d ${BUILD_DIR} ]; then
    mkdir ${BUILD_DIR}
fi

# Create document directory if it doesn't exist
if [ ! -d ${DOCUMENT_DIR} ]; then
    mkdir ${DOCUMENT_DIR}
fi

# Build the project
cd ${BUILD_DIR}
cmake ..
make

# Generate Doxygen documentation
cd ${WORK_DIR}
echo "Generating the documentation..."
doxygen config.doxygen > /dev/null 2>&1
echo "Documentation has been generated"
