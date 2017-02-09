#!/bin/sh

ASMJIT_CURRENT_DIR=`pwd`
ASMJIT_BUILD_DIR="build_xcode"

mkdir ../${ASMJIT_BUILD_DIR}
cd ../${ASMJIT_BUILD_DIR}
cmake .. -G"Xcode" -DASMJIT_BUILD_TEST=1
cd ${ASMJIT_CURRENT_DIR}
