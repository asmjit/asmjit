#!/usr/bin/env sh

set -e

BUILD_DIR=$1

if [ "$USE_VALGRIND" = "1" ]; then
  RUN_CMD="valgrind --leak-check=full --show-reachable=yes --track-origins=yes"
fi

echo ""
echo "=== Starting Tests ==="

echo ""
eval "$RUN_CMD ${BUILD_DIR}/asmjit_test_unit --quick"

echo ""
echo "AsmJit Opcode Test"
eval "$RUN_CMD ${BUILD_DIR}/asmjit_test_opcode > /dev/null"

if [ -f ${BUILD_DIR}/asmjit_test_x86_asm ]; then
  echo ""
  eval "$RUN_CMD ${BUILD_DIR}/asmjit_test_x86_asm"
fi

if [ -f ${BUILD_DIR}/asmjit_test_x86_sections ]; then
  echo ""
  eval "$RUN_CMD ${BUILD_DIR}/asmjit_test_x86_sections"
fi

if [ -f ${BUILD_DIR}/asmjit_test_x86_instinfo ]; then
  echo ""
  eval "$RUN_CMD ${BUILD_DIR}/asmjit_test_x86_instinfo"
fi

if [ -f ${BUILD_DIR}asmjit_test_x86_cc ]; then
  echo ""
  eval "$RUN_CMD ${BUILD_DIR}/asmjit_test_x86_cc"
fi
