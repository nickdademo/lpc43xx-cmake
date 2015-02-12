cmake_minimum_required(VERSION 2.8.4)

project(HelloWorld C)

# Set toolchain file if not set
if(NOT CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE platform/toolchain-gcc-arm-embedded.cmake)
else()
  if(NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Toolchain file does not exist: " ${CMAKE_TOOLCHAIN_FILE})
  endif()
endif()

# Supported C libraries
set(SUPPORTED_CLIBS newlib newlib-nano none redlib)

# Check if C library has been set
if(NOT CLIB)
  message(STATUS "No C library set. Defaulting to newlib.")
  set(CLIB newlib)
else()
    # Check if C library is supported
    if(NOT ";${SUPPORTED_CLIBS};" MATCHES ${CLIB})
      message(FATAL_ERROR "Specified C library is not supported: " ${CLIB})
    endif()
endif()

###########
# LPCOpen #
###########
add_subdirectory(sources/lpcopen/lpc_chip_43xx)

###############
# Application #
###############
add_subdirectory(sources/application)