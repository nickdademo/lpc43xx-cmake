file(GLOB SOURCE_FILES
  *.c
  *.cpp
)
file(GLOB HEADER_FILES
  *.h
)

include_directories(../lpcopen/2.12/lpc_chip_43xx/inc)

if(${CPP})
  add_definitions(-DCPP_USE_HEAP)
endif()

include(${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx.cmake)
include(${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx_targets.cmake)

# Add system files
if(${CPP})
  set(SYSTEM_FILES system/cr_startup_lpc43xx.cpp system/cr_cpp_config.cpp system/crp.c system/sysinit.c)
else()
  set(SYSTEM_FILES system/cr_startup_lpc43xx.c system/crp.c system/sysinit.c)
endif()

add_executable(${OUTPUT_NAME}
  ${SOURCE_FILES}
  ${HEADER_FILES}
  ${SYSTEM_FILES}
)

set_target_properties(${OUTPUT_NAME} PROPERTIES
  LINKER_LANGUAGE ${LANG}
)

if(DEFINED BSP)
  include_directories(../bsp/${BSP}/inc)
  target_link_libraries(${OUTPUT_NAME} ${BSP})
else()
  add_definitions(-DNO_BOARD_LIB)
  target_link_libraries(${OUTPUT_NAME} lpc_chip_43xx)
endif()

# Print size of binary
add_custom_command(
  TARGET ${OUTPUT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_SIZE} ${FULL_OUTPUT_NAME}
)