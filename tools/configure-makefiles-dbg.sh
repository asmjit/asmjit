#!/bin/sh

CURRENT_DIR=`pwd`
BUILD_DIR="build_dbg"

mkdir -p ../${BUILD_DIR}
cd ../${BUILD_DIR}
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DASMJIT_BUILD_SANITIZE=1 -DASMJIT_BUILD_TEST=1
cd ${CURRENT_DIR}
