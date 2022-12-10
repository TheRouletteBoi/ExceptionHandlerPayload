@echo off

set PATH=C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\;%PATH% 

if not exist %PATH%\cmake.exe set PATH=C:\Program Files\CMake\bin\;%PATH%

cmake -Bbuild -GNinja -DCMAKE_MAKE_PROGRAM="%~dp0toolchain\bin\win-x86_64\ninja.exe"
cmake --build build --clean-first

pause
