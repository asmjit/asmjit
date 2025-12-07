@echo off
cmake . -B build_x64 -G "Visual Studio 18 2026" -A x64 -DASMJIT_TEST=ON
