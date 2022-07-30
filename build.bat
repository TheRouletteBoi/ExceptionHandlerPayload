@echo off
cmake -Bbuild -GNinja -DCMAKE_MAKE_PROGRAM="%~dp0toolchain\bin\win-x86_64\ninja.exe"
cmake --build build --clean-first
pause