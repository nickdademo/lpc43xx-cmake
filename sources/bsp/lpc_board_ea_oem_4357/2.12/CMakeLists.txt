set(LIBRARY_NAME lpc_board_ea_oem_4357)

set(LIBRARY_VERSION_MAJOR 2)
set(LIBRARY_VERSION_MINOR 1)
set(LIBRARY_VERSION_PATCH 2)
set(LIBRARY_VERSION ${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}.${LIBRARY_VERSION_PATCH})

set(LPCOPEN_VERSION 2.12)

file(GLOB_RECURSE SOURCE_FILES
  src/*.c
)
file(GLOB_RECURSE HEADER_FILES
  inc/*.h
  src/*.h
)

include_directories(inc)
include_directories(src)
include_directories(../../../lpcopen/{LPCOPEN_VERSION}/lpc_chip_43xx/inc)

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE C)

add_library(${LIBRARY_NAME} STATIC
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

set_target_properties(${LIBRARY_NAME} PROPERTIES
  OUTPUT_NAME "${LIBRARY_NAME}_${LIBRARY_VERSION}"
  LINKER_LANGUAGE C
)

target_link_libraries(${LIBRARY_NAME} lpc_chip_43xx)

include(${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx.cmake)