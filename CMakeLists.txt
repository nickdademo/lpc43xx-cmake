cmake_minimum_required(VERSION 2.8.4)

if(CMAKE_HOST_WIN32)
  set(EXECUTABLE_SUFFIX .exe)
endif()

# Supported C libraries
set(SUPPORTED_CLIBS newlib newlib-nano redlib)

# Supported hosting settings
set(SUPPORTED_HOSTING none nohost semihosting)

# Supported target devices
set(SUPPORTED_DEVICES LPC4357)

# Check if C++ is enabled
if(CPP)
  set(LANG "CXX")
else()
  set(LANG "C")
endif()
message(STATUS "Target language: " ${LANG})

# Set toolchain file if not specified
if(NOT CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE platform/toolchain-gcc-arm-embedded.cmake)
else()
  if(NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Toolchain file does not exist: " ${CMAKE_TOOLCHAIN_FILE})
  endif()
endif()
message(STATUS "Toolchain file: " ${CMAKE_TOOLCHAIN_FILE})

# Set Eclipse make arguments if not specified
if(NOT CMAKE_ECLIPSE_MAKE_ARGUMENTS)
  set(CMAKE_ECLIPSE_MAKE_ARGUMENTS "-j4") # Parallel building
endif()
message(STATUS "Eclipse make arguments: " ${CMAKE_ECLIPSE_MAKE_ARGUMENTS})

project(HelloWorld)

# Supported build configurations: "Release" and "Debug"
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
  "Reset the configurations to what we need"
  FORCE
)

# Set default build type if not specified
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
else()
  if(NOT ";${CMAKE_CONFIGURATION_TYPES};" MATCHESL ${CMAKE_BUILD_TYPE})
    message(FATAL_ERROR "Specified build type is not supported: " ${CMAKE_BUILD_TYPE})
  endif()
endif()
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

# Set C/C++ library if not specified
if(NOT CLIB)
  set(CLIB newlib-nano)
else()
    # Check if library is supported
    if(NOT ";${SUPPORTED_CLIBS};" MATCHES ${CLIB})
      message(FATAL_ERROR "Specified library is not supported: " ${CLIB})
    endif()
    # Check if language/library combination is supported
    if(${CPP} AND ${CLIB} STREQUAL redlib)
      message(FATAL_ERROR "Specified library does not support C++: " ${CLIB})
    endif()
endif()
message(STATUS "C/C++ library: " ${CLIB})

# Check if Code Read Protection is enabled
if(CRP)
  message(STATUS "Code Read Protection: Enabled")
else()
  message(STATUS "Code Read Protection: Disabled")
endif()

# Set hosting if not specified
if(NOT HOSTING)
  set(HOSTING nohost)
else()
    # Check if C library is supported
    if(NOT ";${SUPPORTED_HOSTING};" STREQUAL ${HOSTING})
      message(FATAL_ERROR "Specified hosting setting is not supported: " ${HOSTING})
    endif()
endif()
message(STATUS "Hosting setting: " ${HOSTING})

# Set hosting if not specified
if(NOT DEVICE)
  set(DEVICE LPC4357)
else()
    # Check if device is supported
    if(NOT ";${SUPPORTED_DEVICES};" STREQUAL ${DEVICE})
      message(FATAL_ERROR "Specified device is not supported: " ${DEVICE})
    endif()
endif()
message(STATUS "Target device: " ${DEVICE})

# Set flash driver file if not specified
if(NOT FLASHDRIVER)
  set(FLASHDRIVER LPC18x7_43x7_2x512_BootA.cfx)
endif()
message(STATUS "LPCXpresso flash driver file: " ${FLASHDRIVER})

# Set reset script if not specified
if(NOT RESETSCRIPT)
  set(RESETSCRIPT LPC18LPC43InternalFLASHBootResetscript.scp)
endif()
message(STATUS "LPCXpresso reset script: " ${RESETSCRIPT})

# Check if Python is present
find_package(PythonInterp 3.4)
if(NOT PYTHONINTERP_FOUND)
  message(STATUS "WARNING: Python could not be found. Python is required for debugging within LPCXpresso.")
else()
  message(STATUS "Python found: " ${PYTHON_EXECUTABLE})
endif()

# Check if LPCXpresso checksum tool is present
if(EXISTS ${TOOLCHAIN_PREFIX}/../bin/checksum${EXECUTABLE_SUFFIX})
  set(CHECKSUM_TOOL ${TOOLCHAIN_PREFIX}/../bin/checksum)
  set(HAVE_CHECKSUM_TOOL ON)
  message(STATUS "LPCXpresso checksum tool found: " ${CHECKSUM_TOOL})
  message(STATUS "'bin' make target will add checksum information.")
else()
  set(HAVE_CHECKSUM_TOOL OFF)
  message(STATUS "WARNING: LPCXpresso checksum tool not found. 'bin' make target will NOT add checksum information.")
endif()

# Check if specified path to the OpenOCD binary is correct
if(EXISTS ${OPENOCD_BINARY})
  if(${HAVE_CHECKSUM_TOOL})
    # Configure OpenOCD flashing depending on build settings
    if(FLASHDRIVER STREQUAL LPC18x7_43x7_2x512_BootA.cfx)
      set(FLASH_BANK 0)
      set(FLASH_BANK_ADDRESS 0x1a000000)
    elseif(FLASHDRIVER STREQUAL LPC18x7_43x7_2x512_BootB.cfx)
      set(FLASH_BANK 1)
      set(FLASH_BANK_ADDRESS 0x1b000000)
    else()
      message(STATUS "WARNING: OpenOCD does not support this flash driver: " ${FLASHDRIVER})
      message(STATUS "'make flash' and 'make erase' targets will NOT be available.")
    endif()
    set(HAVE_OPENOCD ON)
    message(STATUS "OpenOCD found: " ${OPENOCD_BINARY})
    message(STATUS "'make flash' and 'make erase' targets will be available.")
    # Set OpenOCD configuration file if not specified
    if(NOT DEFINED OPENOCD_CONFIG)
      set(OPENOCD_CONFIG stlink-v2_lpc43xx.cfg)
    endif()
    message(STATUS "OpenOCD configuration: " ${OPENOCD_CONFIG})
    # Set OpenOCD transport if not specified
    if(NOT DEFINED OPENOCD_TRANSPORT)
      set(OPENOCD_TRANSPORT hla_jtag)
    endif()
    message(STATUS "OpenOCD transport: " ${OPENOCD_TRANSPORT})
  else()
    set(HAVE_OPENOCD OFF)
    message(STATUS "WARNING: LPCXpresso checksum tool not found. 'make flash' and 'make erase' targets will NOT be available.")
  endif()
else()
  set(HAVE_OPENOCD OFF)
  message(STATUS "WARNING: OpenOCD path not specified. 'make flash' and 'make erase' targets will NOT be available.")
endif()

# Store paths to commonly used files/locations for convenience
set(APP_PATH ${CMAKE_SOURCE_DIR}/sources/application)
set(PLATFORM_FILE ${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx.cmake)
set(PLATFORM_TARGETS ${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx_targets.cmake)

###########
# LPCOpen #
###########
if(NOT DEFINED LPCOPEN_VERSION)
  set(LPCOPEN_VERSION 2.12)
endif()
set(LPCOPEN_PATH ${CMAKE_SOURCE_DIR}/sources/lpcopen/${LPCOPEN_VERSION}/lpc_chip_43xx)
add_subdirectory(${LPCOPEN_PATH})
message(STATUS "LPCOpen Path: " ${LPCOPEN_PATH})
message(STATUS "LPCOpen Version: " ${LPCOPEN_VERSION})

#########################
# Board Support Package #
#########################
if(NOT DEFINED BSP_NAME AND NOT DEFINED BSP_VERSION)
  message(STATUS "No BSP specified. BSP will NOT be included in build.")
  add_definitions(-DNO_BOARD_LIB)
else()
  if(NOT DEFINED BSP_NAME)
    message(STATUS "WARNING: No BSP Name specified.")
  endif()
  if(NOT DEFINED BSP_VERSION)
    message(STATUS "WARNING: No BSP Version specified.")
  endif()
  set(BSP_PATH ${CMAKE_SOURCE_DIR}/sources/bsp/${BSP_NAME}/${BSP_VERSION})
  # Check if path exists
  if(NOT EXISTS ${BSP_PATH})
    message(FATAL_ERROR "BSP path is invalid: " ${BSP_PATH})
  endif()
  add_subdirectory(${BSP_PATH})
  message(STATUS "Board Support Package: " ${BSP_NAME})
  message(STATUS "Board Support Version: " ${BSP_VERSION})
endif()

###############
# Application #
###############
add_subdirectory(sources/application)

# Print linker script used
message(STATUS "Linker script: " ${LINKER_SCRIPT})

# Check whether *printf float and char printf arguments are applicable
if(${PRINTF_FLOAT})
  if(NOT ${CLIB} STREQUAL newlib-nano OR NOT ${CLIB} STREQUAL redlib)
    message(STATUS "WARNING: printf float argument not valid for library: " ${CLIB})
  endif()
elseif(NOT ${PRINTF_FLOAT})
  if(NOT ${CLIB} STREQUAL newlib-nano)
    message(STATUS "WARNING: printf float argument not valid for library: " ${CLIB})
  endif()
endif()
if(${SPRINTF_FLOAT})
  if(NOT ${CLIB} STREQUAL newlib-nano)
    message(STATUS "WARNING: sprintf float argument not valid for library: " ${CLIB})
  endif()
elseif(NOT ${SPRINTF_FLOAT})
  if(NOT ${CLIB} STREQUAL newlib-nano)
    message(STATUS "WARNING: sprintf float argument not valid for library: " ${CLIB})
  endif()
endif()
if(${CHAR_PRINTF})
  if(NOT ${CLIB} STREQUAL redlib)
    message(STATUS "WARNING: char printf argument not valid for library: " ${CLIB})
  endif()
elseif(NOT ${CHAR_PRINTF})
  if(NOT ${CLIB} STREQUAL redlib)
    message(STATUS "WARNING: char printf argument not valid for library: " ${CLIB})
  endif()
endif()

if(${CLIB} STREQUAL newlib-nano OR ${CLIB} STREQUAL redlib)
  # printf float (newlib-nano and redlib only)
  if(${PRINTF_FLOAT})
    message(STATUS "printf float enabled.")
  else()
    message(STATUS "printf float disabled.")
  endif()
endif()
if(${CLIB} STREQUAL newlib-nano)
  # sprintf float (newlib-nano only)
  if(${SPRINTF_FLOAT})
    message(STATUS "sprintf float enabled.")
  else()
    message(STATUS "sprintf float disabled.")
  endif()
endif()
if(${CLIB} STREQUAL redlib)
  # char printf (redlib only)
  if(${CHAR_PRINTF})
    message(STATUS "char printf enabled.")
  else()
    message(STATUS "char printf disabled.")
  endif()
endif()

#####################################
# Eclipse .cproject patching script #
#####################################
if(PYTHONINTERP_FOUND)
  # Windows
  if (CMAKE_HOST_WIN32)
    set(OUT patch_cproject.bat)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E echo "${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/patch_cproject.py --pythonpath ${PYTHON_EXECUTABLE} --scriptpath ${PROJECT_SOURCE_DIR}/scripts --cproject ${PROJECT_BINARY_DIR}/.cproject --device ${DEVICE} --flashdriver ${FLASHDRIVER} --resetscript ${RESETSCRIPT}"
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      OUTPUT_QUIET
      OUTPUT_FILE ${OUT}
    )
    message(STATUS "Eclipse .cproject patching script generated: " ${OUT})
  # Linux / MacOS
  else()
    set(OUT patch_cproject.sh)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E echo "#!/bin/bash\n${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/patch_cproject.py --pythonpath ${PYTHON_EXECUTABLE} --scriptpath ${PROJECT_SOURCE_DIR}/scripts --cproject ${PROJECT_BINARY_DIR}/.cproject --device ${DEVICE} --flashdriver ${FLASHDRIVER} --resetscript ${RESETSCRIPT}"
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      OUTPUT_QUIET
      OUTPUT_FILE ${OUT}
    )
    execute_process(
      COMMAND chmod +x ${OUT}
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      OUTPUT_QUIET
    )
    message(STATUS "Eclipse .cproject patching script generated: " ${OUT})
  endif()
else()
  message(STATUS "WARNING: Eclipse .cproject patching script NOT generated.")
endif()
