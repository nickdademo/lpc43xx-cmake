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

# LPCOpen
add_subdirectory(sources/lpcopen/lpc_chip_43xx)

# Application
add_subdirectory(sources/application)