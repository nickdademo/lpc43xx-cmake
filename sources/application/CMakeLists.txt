file(GLOB SOURCE_FILES
  *.c
  system/*.c
)
file(GLOB HEADER_FILES
  *.h
  system/*.h
)

include_directories(../lpcopen/lpc_chip_43xx/inc)

add_definitions("-DNO_BOARD_LIB")

include(${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx.cmake)
include(${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx_targets.cmake)

add_executable(${OUTPUT_NAME}
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

set_target_properties(${OUTPUT_NAME} PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(${OUTPUT_NAME} lpc_chip_43xx)

# Separate debug symbols into their own file
add_custom_command(
  TARGET ${OUTPUT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} --only-keep-debug ${FULL_OUTPUT_NAME} ${FULL_OUTPUT_NAME}.debug
  COMMAND ${CMAKE_STRIP} -g ${FULL_OUTPUT_NAME}
)