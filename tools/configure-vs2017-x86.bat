@echo off

set CURRENT_DIR=%CD%
set BUILD_DIR="build_vs2017_x86"

mkdir ..\%BUILD_DIR%
cd ..\%BUILD_DIR%
cmake .. -G"Visual Studio 15" -DASMJIT_BUILD_TEST=1
cd %CURRENT_DIR%
