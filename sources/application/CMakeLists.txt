file(GLOB SOURCE_FILES
  *.c
  system/*.c
)
file(GLOB HEADER_FILES
  *.h
  system/*.h
)

include_directories(../lpcopen/lpc_chip_43xx/inc)

add_definitions(-DNO_BOARD_LIB)

include(${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx.cmake)
include(${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx_targets.cmake)

add_executable(${OUTPUT_NAME}
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

set_target_properties(${OUTPUT_NAME} PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(${OUTPUT_NAME} lpc_chip_43xx)

# Print size of binary
add_custom_command(
  TARGET ${OUTPUT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_SIZE} ${FULL_OUTPUT_NAME}
)