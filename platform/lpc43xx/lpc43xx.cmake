# Common definitions
add_definitions(
  -DCORE_M4
  -D__USE_LPCOPEN
  -D__CODE_RED
  -D__LPC43XX__
  -D__MULTICORE_NONE
  -c
)

# Build type-specific definitions
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_definitions(-O2 -Os)          # O2: Optimize even more, Os: Optimize for size
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-O0 -g3 -DDEBUG)  # O0: Reduce compilation time and make debugging produce the expected results (default), g3: Level 3 debugging information
endif()

# C flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fmessage-length=0 -ffunction-sections -fdata-sections -fsingle-precision-constant -fno-builtin -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp")
# C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions")

# Dynamically create linker script
if(${CPP})
  set(LINKER_SCRIPT_SECTIONS_NAME "cpp/sections")
else()
  set(LINKER_SCRIPT_SECTIONS_NAME "sections")
endif()
if(${CRP})
  set(LINKER_SCRIPT_SECTIONS_NAME "${LINKER_SCRIPT_SECTIONS_NAME}_crp")
endif()
set(LINKER_SCRIPT_SECTIONS sections/${LINKER_SCRIPT_SECTIONS_NAME}.ld CACHE INTERNAL "Linker script SECTIONS component")
set(LINKER_SCRIPT_LIB lib/${CLIB}/${HOSTING}_lib.ld CACHE INTERNAL "Linker script LIB component")
set(LINKER_SCRIPT_LIB_CPP "" CACHE INTERNAL "Linker script LIB CPP component")
set(LINKER_SCRIPT_MEM mem/mem.ld CACHE INTERNAL "Linker script MEM component")
set(LINKER_SCRIPT_FILENAME ${CMAKE_PROJECT_NAME}.ld)

# Check that all files exist
if(NOT EXISTS ${LINKER_SCRIPT_DIR}/${LINKER_SCRIPT_SECTIONS})
  message(FATAL_ERROR "Linker script SECTIONS component cannot be found at: " ${LINKER_SCRIPT_DIR}/${LINKER_SCRIPT_SECTIONS})
endif()
if(NOT EXISTS ${LINKER_SCRIPT_DIR}/${LINKER_SCRIPT_LIB})
  message(FATAL_ERROR "Linker script LIB component cannot be found at: " ${LINKER_SCRIPT_DIR}/${LINKER_SCRIPT_LIB})
endif()
if(NOT EXISTS ${LINKER_SCRIPT_DIR}/${LINKER_SCRIPT_MEM})
  message(FATAL_ERROR "Linker script MEM component cannot be found at: " ${LINKER_SCRIPT_DIR}/${LINKER_SCRIPT_MEM})
endif()

if(CPP AND (${CLIB} STREQUAL newlib OR ${CLIB} STREQUAL newlib-nano))
  set(LINKER_SCRIPT_LIB_CPP lib/${CLIB}/lib_cpp.ld CACHE INTERNAL "Linker script LIB CPP component")
  if(NOT EXISTS ${LINKER_SCRIPT_DIR}/${LINKER_SCRIPT_LIB_CPP})
    message(FATAL_ERROR "Linker script LIB CPP component cannot be found at: " ${LINKER_SCRIPT_DIR}/${LINKER_SCRIPT_LIB_CPP})
  endif()
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E echo "INCLUDE \"${LINKER_SCRIPT_LIB}\"\nINCLUDE \"${LINKER_SCRIPT_LIB_CPP}\"\nINCLUDE \"${LINKER_SCRIPT_MEM}\"\nINCLUDE \"${LINKER_SCRIPT_SECTIONS}\""
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    OUTPUT_QUIET
    OUTPUT_FILE ${LINKER_SCRIPT_FILENAME}
  )
else()
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E echo "INCLUDE \"${LINKER_SCRIPT_LIB}\"\nINCLUDE \"${LINKER_SCRIPT_MEM}\"\nINCLUDE \"${LINKER_SCRIPT_SECTIONS}\""
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    OUTPUT_QUIET
    OUTPUT_FILE ${LINKER_SCRIPT_FILENAME}
  )
endif()

# Set specs argument and definitions based on specified C/C++ library
# newlib
if(${CLIB} STREQUAL newlib)
  add_definitions(-D__NEWLIB__)
# newlib-nano
elseif(${CLIB} STREQUAL newlib-nano)
  # printf float
  if(${PRINTF_FLOAT})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u _printf_float")
  endif()
  # sprintf float
  if(${SPRINTF_FLOAT})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u _sprintf_float")
  endif()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")
  add_definitions(-D__NEWLIB__ -specs=nano.specs)
# redlib
elseif(${CLIB} STREQUAL redlib)
  # printf float
  if(NOT ${PRINTF_FLOAT})
    add_definitions(-DCR_INTEGER_PRINTF)
  endif()
  # character-based printf (rather than string-based)
  if(${CHAR_PRINTF})
    add_definitions(-DCR_PRINTF_CHAR)
  endif()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=redlib.specs")
  add_definitions(-D__REDLIB__ -specs=redlib.specs)
endif()

set(LINKER_SCRIPT ${PROJECT_BINARY_DIR}/${LINKER_SCRIPT_FILENAME} CACHE INTERNAL "Linker script")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -Xlinker --gc-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -T ${LINKER_SCRIPT} -L ${LINKER_SCRIPT_DIR}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(FULL_OUTPUT_NAME ${OUTPUT_NAME}.axf)
set(FULL_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/${FULL_OUTPUT_NAME})