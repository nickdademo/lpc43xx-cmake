set(LIBRARY_NAME lpc_chip_43xx)

file(GLOB_RECURSE SOURCE_FILES src/*.c)
file(GLOB_RECURSE HEADER_FILES inc/*.h)

include_directories(inc)

add_library(${LIBRARY_NAME} STATIC
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

include(${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx.cmake)