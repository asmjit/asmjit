@echo off

set ASMJIT_CURRENT_DIR=%CD%
set ASMJIT_BUILD_DIR="build_vs2010_x64"

mkdir ..\%ASMJIT_BUILD_DIR%
cd ..\%ASMJIT_BUILD_DIR%
cmake .. -G"Visual Studio 10 Win64" -DASMJIT_BUILD_TEST=1
cd %ASMJIT_CURRENT_DIR%
