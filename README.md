# lpc43xx-cmake
## Prerequisites
*Versions listed are verified as working.*
* CMake (v3.1.2): http://www.cmake.org/
* GNU ARM Toolchain:
 * OPTION 1) GCC ARM Embedded (v4.9 2014 Q4): https://launchpad.net/gcc-arm-embedded
 * OPTION 2) LPCXpresso (v7.6.2): http://www.lpcware.com/lpcxpresso/download
* (Windows-only) GNU ARM Build Tools (v2.3): http://sourceforge.net/projects/gnuarmeclipse/files/Build%20Tools/
 * Ensure the path to the installed tools is added to the Windows PATH variable. 

## Building
1. Clone this repository.
2. Create a new folder next to the cloned repository.
3. Run CMake within the newly created folder to generate Makefiles and Eclipse project files (specify the correct path to the GNU ARM toolchain):  
*$ cmake -DTOOLCHAIN_PREFIX="C:/nxp/LPCXpresso_7.6.2_326/lpcxpresso/tools" -G "Eclipse CDT4 - Unix Makefiles" ../lpc43xx-cmake*  
4. Build project:  
*$ make*

## CMake Arguments
A number of additional arguments can be specified to CMake to further configure the build:
* **TOOLCHAIN_PREFIX**: Path to the GNU ARM toolchain.
* **CMAKE_BUILD_TYPE**: Build type [Release, Debug]. *Default: Release*
* **CMAKE_ECLIPSE_MAKE_ARGUMENTS**: Arguments to pass to Eclipse make. Here the number of threads for parallel building can be specified. *Default: -j4*
* **CMAKE_ECLIPSE_VERSION**: Version of Eclipse to generate for. Specifying the correct version allows the CMake Eclipse Generatort to use the latest Eclipse features. *LPCXpresso v7.6.2 uses Eclipse 4.4.0.*
* **CLIB**: C/C++ library to use [newlib, newlib-nano, redlib]. *Default: newlib-nano*
* **HOSTING**: Hosting settings for the build [none, nohost, semihosting]. *Default: nohost*
