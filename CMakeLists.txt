cmake_minimum_required(VERSION 2.8.4)

project(HelloWorld C)

# LPCOpen
add_subdirectory(sources/lpcopen/lpc_chip_43xx)

# Application
add_subdirectory(sources/application)