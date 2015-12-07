@echo off

set ASMJIT_CURRENT_DIR=%CD%
set ASMJIT_BUILD_DIR="build_mingw_dbg"

mkdir ..\%ASMJIT_BUILD_DIR%
cd ..\%ASMJIT_BUILD_DIR%
cmake .. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DASMJIT_BUILD_TEST=1
cd %ASMJIT_CURRENT_DIR%
