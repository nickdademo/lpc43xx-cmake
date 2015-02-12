cmake_minimum_required(VERSION 2.8.4)

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

project(HelloWorld C)

# Supported C libraries
set(SUPPORTED_CLIBS newlib newlib-nano redlib)

# Supported hosting settings
set(SUPPORTED_HOSTING none nohost semihosting)

# Supported build configurations: "Release" and "Debug"
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
  "Reset the configurations to what we need"
  FORCE
)

# Set default build type if not specified
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
else()
  if(NOT ";${CMAKE_CONFIGURATION_TYPES};" MATCHES ${CMAKE_BUILD_TYPE})
    message(FATAL_ERROR "Specified build type is not supported: " ${CMAKE_BUILD_TYPE})
  endif()
endif()
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

# Set C library if not specified
if(NOT CLIB)
  set(CLIB newlib-nano)
else()
    # Check if C library is supported
    if(NOT ";${SUPPORTED_CLIBS};" MATCHES ${CLIB})
      message(FATAL_ERROR "Specified C library is not supported: " ${CLIB})
    endif()
endif()
message(STATUS "C library: " ${CLIB})

# Set hosting if not specified
if(NOT HOSTING)
  set(HOSTING nohost)
else()
    # Check if C library is supported
    if(NOT ";${SUPPORTED_HOSTING};" MATCHES ${HOSTING})
      message(FATAL_ERROR "Specified hosting setting is not supported: " ${HOSTING})
    endif()
endif()
message(STATUS "Hosting setting: " ${HOSTING})

###########
# LPCOpen #
###########
add_subdirectory(sources/lpcopen/lpc_chip_43xx)

###############
# Application #
###############
add_subdirectory(sources/application)