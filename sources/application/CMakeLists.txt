file(GLOB SOURCE_FILES
  *.c
  *.cpp
)
file(GLOB HEADER_FILES
  *.h
)

include_directories(${LPCOPEN_PATH}/inc)

if(${CPP})
  add_definitions(-DCPP_USE_HEAP)
endif()

include(${PLATFORM_FILE})
include(${PLATFORM_TARGETS})

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

if(DEFINED BSP_PATH)
  include_directories(${BSP_PATH}/inc)
  target_link_libraries(${OUTPUT_NAME} ${BSP_NAME})
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