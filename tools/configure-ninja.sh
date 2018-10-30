#!/bin/sh

BUILD_DIR="build"
BUILD_OPTIONS="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DASMJIT_BUILD_TEST=1"

CURRENT_DIR=`pwd`
mkdir -p ../${BUILD_DIR}_dbg
mkdir -p ../${BUILD_DIR}_rel

cd ../${BUILD_DIR}_dbg
eval cmake .. -G"Ninja" -DCMAKE_BUILD_TYPE=Debug ${BUILD_OPTIONS} -DASMJIT_BUILD_SANITIZE=1
cd ${CURRENT_DIR}

cd ../${BUILD_DIR}_rel
eval cmake .. -G"Ninja" -DCMAKE_BUILD_TYPE=Release ${BUILD_OPTIONS}
cd ${CURRENT_DIR}
