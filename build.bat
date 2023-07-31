@echo off
setlocal

:: find CMake in Program Files
set "PATH=C:\Program Files\CMake\bin;%PATH%"

:: search for CMake in Visual Studio installation path 
if not exist "%PATH%\cmake.exe" (
    for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -property installationPath`) do (
        set "installDir=%%i"
        set "PATH=%installDir%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
    )
)

:: If CMake is not found, use the hardcoded path for Visual Studio 2022 Preview Edition
if not exist "%PATH%\cmake.exe" (
    set "PATH=C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
)

:: Set the path to Ninja
set "NINJA_PATH=%~dp0toolchain\bin\win-x86_64\ninja.exe"

:: Execute CMake with Ninja generator
cmake -Bbuild -GNinja -DCMAKE_MAKE_PROGRAM="%NINJA_PATH%"
cmake --build build --clean-first

pause
