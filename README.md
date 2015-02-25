# lpc43xx-cmake
## Prerequisites
*Versions listed are verified as working.*
* CMake (v3.1.2): http://www.cmake.org/
* GNU ARM Toolchain:
 * LPCXpresso (v7.6.2): http://www.lpcware.com/lpcxpresso/download
* (Windows-only) GNU ARM Build Tools (v2.3): http://sourceforge.net/projects/gnuarmeclipse/files/Build%20Tools/
 * Ensure the path to the installed tools is added to the Windows `PATH` variable.
 * Alternatively, instead of installing the GNU ARM Built Tools, you can also simply add the path to the LPCXpresso-supplied tools to the Windows `PATH` variable: e.g. *C:\nxp\LPCXpresso_7.6.2_326\lpcxpresso\msys\bin*
* Python 3.4+ (v3.4.2) (if debugging within LPCXpresso is required): https://www.python.org/downloads/
 * Requires lxml package: http://lxml.de/

## Building
1) Clone this repository.  
2) Create a new folder next to the cloned repository (e.g. 'lpc43xx-cmake_build').  
3) Run CMake within the newly created folder to generate Makefiles and Eclipse project files (specify the correct path to the GNU ARM toolchain):  
```
$ cmake -DTOOLCHAIN_PREFIX="C:/nxp/LPCXpresso_7.6.2_326/lpcxpresso/tools" -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_VERSION=4.4.0 ../lpc43xx-cmake
```
4) Build project:  
```
$ make
```
5) **(OPTIONAL: If debugging within LPCXpresso is required)** Patch generated Eclipse .cproject file:  

Windows:
```
$ patch_cproject.bat
```
Linux/Mac:
```
$ patch_cproject.sh
```
**IMPORTANT:** Patch must be re-applied if CMake is re-run.

## CMake Arguments
A number of additional arguments can be specified to CMake to further configure the build:
* **TOOLCHAIN_PREFIX**: Path to the GNU ARM toolchain.
* **CMAKE_BUILD_TYPE**: Build type [Release, Debug]. *Default: Debug*
* **CMAKE_ECLIPSE_MAKE_ARGUMENTS**: Arguments to pass to Eclipse make. Here the number of threads for parallel building can be specified. *Default: -j4*
* **CMAKE_ECLIPSE_VERSION**: Version of Eclipse to generate for. Specifying the correct version allows the CMake Eclipse Generator to use the latest Eclipse features. *LPCXpresso v7.6.2 uses Eclipse 4.4.0.*
* **CLIB**: C/C++ library to use [newlib, newlib-nano, redlib]. *Default: newlib-nano*
* **HOSTING**: Hosting settings for the build [none, nohost, semihosting]. *Default: nohost*
* **DEVICE**: Target device name [LPC4357]. *Default: LPC4357*
* **FLASHDRIVER**: LPCXpresso-supplied file which determines where the program will be flashed to and booted from. *Default: LPC18x7_43x7_2x512_BootA.cfx*
* **RESETSCRIPT**: LPCXpresso-supplied file used during debugging. *Default: LPC18LPC43InternalFLASHBootResetscript.scp*
* **PRINTF_FLOAT**: Enable/disable float format in printf (for newlib-nano and redlib only) [ON, OFF]. *Default: OFF*
* **SPRINTF_FLOAT**: Enable/disable float format in sprintf (for newlib-nano only) [ON, OFF]. *Default: OFF*
* **CHAR_PRINTF**: Enable/disable character-based printf (rather than string-based) (for redlib only) [ON, OFF]. *Default: OFF*
* **CPP**: Enable/disable C++ support [ON, OFF]. *Default: OFF*
* **CRP**: Enable/disable Code Read Protection [ON, OFF]. *Default: OFF*
* **BSP_NAME**: Board Support Package to use. *This argument is optional: just ensure that the path to the BSP matches sources/bsp/```BSP_NAME```/```BSP_VERSION```. Note: If both ```BSP_NAME``` and ```BSP_VERSION``` are not specified, no BSP will be used.*
* **BSP_VERSION**: Board Support Package version to use. *This argument is optional: just ensure that the path to the BSP matches sources/bsp/```BSP_NAME```/```BSP_VERSION```. Note: If both ```BSP_NAME``` and ```BSP_VERSION``` are not specified, no BSP will be used.*
* **LPCOPEN_VERSION**: LPCOpen version to use in the BSP (or in the application if no BSP is present). The value must match the name of a subdirectory in the *sources/lpcopen* folder. *Default: 2.12*
* **OPENOCD_BINARY**: Specify the path to the OpenOCD binary executable. If specified and valid, target device can be flashed via ```make flash```.
* **OPENOCD_CONFIG**: Specify the OpenOCD configuration file to use. This value must match the name of a file in the *debug* directory. *Default: stlink-v2_lpc43xx.cfg*
* **OPENOCD_TRANSPORT**: Specify the transport to use with the OpenOCD configuration. Ensure the configuration supports the transport you specify. *Default: hla_jtag*

## Make Targets
* ```make```: Builds the entire project and outputs an .axf (ARM Executable Format) file - this is actually a ELF/DWARF image. Equivalent to ```make all```.
* ```make <target_name>```: Build only a specific target (e.g: ```make lpc_chip_43xx```)
* ```make clean```: Cleans all target output files
* ```make hex```: Generate a hex file
* ```make bin```: Generate a binary file which includes the required checksum (See: http://www.lpcware.com/content/faq/lpcxpresso/image-checksums)
* ```make lst```: Generate a listings file
* ```make flash```: Flash the binary file to the target via OpenOCD. Only available if the CMake argument ```OPENOCD_BINARY``` is specified and valid. Note: This target calls ```make bin``` before flashing.
* ```make erase```: Erases the target device flash via OpenOCD. Only available if the CMake argument ```OPENOCD_BINARY``` is specified and valid. Note: The flash bank specified by ```FLASHDRIVER``` will be erased only.
