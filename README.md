# ExceptionHandlerPayload
 Automatically quit to XMB if a GameOS thread has crashed
 
 ## Features
 - WIP: Automatically quit to XMB if a GameOS thread has crashed
 - WIP: Ability to use DEX Menus on HEN/CEX by redirecting syscalls
 - TODO: Dump thread exceptions to file
 - TODO: Override EBOOT path for your own path
 - TODO: Override SPRX path for you own path
 

## Installation
1. In your PS3 file system go to `/dev_hdd0/plugins/kernel` and transfer file named `ExceptionHandlerPayload.krnl` provided in Releases

2. Navigate back to /dev_hdd0 and you will find a file named `boot_plugins_kernel.txt` (If file is not found just create one named boot_plugins_kernel.txt)

3. Open boot_plugins_kernel.txt and make a new line and type `/dev_hdd0/plugins/kernel/ExceptionHandlerPayload.krnl`

4. Restart your console


## Download 
[Releases](https://github.com/TheRouletteBoi/ExceptionHandlerPayload/releases)


## How to build

Requirements:

 * [CMake](https://cmake.org/download/)
 
### Building on Windows
 Simply run build.bat
 
### Building with VS Code
 Open the folder in VS Code and press `Ctrl+Shift+B` to build or deploy the payload into console.
 Change the console IP in .vscode/settings to yours, and be sure you have a ftp server running on console (e.g webMAN).


## Credits
 [@iMoD1998](https://github.com/iMoD1998) for kernel payload C++ template
