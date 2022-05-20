

set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          powerpc64)

# Without that flag CMake is not able to pass test compilation check
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

set(PS3_PPU_TOOLCHAIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/toolchain/bin/)
set(triple powerpc64-eabi-freebsd-elfv1)

set(CMAKE_LINKER ${PS3_PPU_TOOLCHAIN_PATH}ld${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_AR ${PS3_PPU_TOOLCHAIN_PATH}llvm-ar)
set(CMAKE_OBJCOPY ${PS3_PPU_TOOLCHAIN_PATH}llvm-objcopy)

SET(CMAKE_ASM_COMPILER ${PS3_PPU_TOOLCHAIN_PATH}clang.exe)
set(CMAKE_C_COMPILER ${PS3_PPU_TOOLCHAIN_PATH}clang.exe)
set(CMAKE_CXX_COMPILER ${PS3_PPU_TOOLCHAIN_PATH}clang++.exe)

set(CMAKE_EXE_LINKER_FLAGS "-Xlinker -v -nodefaultlibs -nostartfiles")
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})
SET(CMAKE_ASM_COMPILER_TARGET ${triple})

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -B\"${PS3_PPU_TOOLCHAIN_PATH}\"")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)