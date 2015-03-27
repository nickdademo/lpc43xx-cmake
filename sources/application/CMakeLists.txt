file(GLOB SOURCE_FILES
  *.c
  *.cpp
)
file(GLOB HEADER_FILES
  *.h
)

include_directories(${LPCOPEN_CHIP_INCLUDE_PATHS})

# Add system files
if(${CPP})
  set(SYSTEM_FILES ${SYSTEM_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/system/crp.c)
  set(SYSTEM_FILES ${SYSTEM_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/system/sysinit.c)
  set(SYSTEM_FILES ${SYSTEM_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/system/cr_startup_lpc43xx.cpp)
  set(SYSTEM_FILES ${SYSTEM_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/system/cr_cpp_config.cpp)
  list(APPEND SOURCE_FILES ${SYSTEM_FILES})
else()
  set(SYSTEM_FILES ${SYSTEM_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/system/crp.c)
  set(SYSTEM_FILES ${SYSTEM_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/system/sysinit.c)
  set(SYSTEM_FILES ${SYSTEM_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/system/cr_startup_lpc43xx.c)
  list(APPEND SOURCE_FILES ${SYSTEM_FILES})
endif()

if(${CPP})
  add_definitions(-DCPP_USE_HEAP)
endif()

include(${PLATFORM_FILE})
include(${PLATFORM_TARGETS})

add_executable(${FULL_OUTPUT_NAME}
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

set_target_properties(${FULL_OUTPUT_NAME} PROPERTIES
  LINKER_LANGUAGE ${LANG}
)

if(DEFINED BSP_PATH)
  include_directories(${BSP_INCLUDE_PATHS})
  target_link_libraries(${FULL_OUTPUT_NAME} ${BSP_LIBRARY_NAME})
else()
  target_link_libraries(${FULL_OUTPUT_NAME} lpc_chip_43xx)
endif()

# Print size of binary
add_custom_command(
  TARGET ${FULL_OUTPUT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_SIZE} ${FULL_OUTPUT_PATH}
)
