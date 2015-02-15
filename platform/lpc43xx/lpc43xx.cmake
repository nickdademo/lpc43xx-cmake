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
if(DEFINED CMAKE_RELEASE)
  add_definitions(-O2 -Os)          # O2: Optimize even more, Os: Optimize for size
else()
  add_definitions(-O0 -g3 -DDEBUG)  # O0: Reduce compilation time and make debugging produce the expected results, g3: Level 3 debugging information
endif()

# C flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fmessage-length=0 -ffunction-sections -fdata-sections -fsingle-precision-constant -fno-builtin -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp")
# C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions")

set(LINKER_SCRIPT_DIR "${CMAKE_SOURCE_DIR}/platform/lpc43xx/ldscripts")

# Dynamically create linker script
if(${CPP})
  set(LINKER_SCRIPT_BASENAME "cpp/base")
else()
  set(LINKER_SCRIPT_BASENAME "base")
endif()
if(${CRP})
  set(LINKER_SCRIPT_BASENAME "${LINKER_SCRIPT_BASENAME}_crp")
endif()
set(LINKER_SCRIPT_BASE base/${LINKER_SCRIPT_BASENAME}.ld)
set(LINKER_SCRIPT_LIB lib/${CLIB}/${HOSTING}_lib.ld)
set(LINKER_SCRIPT_MEM mem/mem.ld)
set(LINKER_SCRIPT_FILENAME ${CMAKE_PROJECT_NAME}.ld)

if(${CLIB} STREQUAL newlib OR ${CLIB} STREQUAL newlib-nano)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E echo "INCLUDE \"${LINKER_SCRIPT_LIB}\"\nINCLUDE \"lib/${CLIB}/lib_cpp.ld\"\nINCLUDE \"${LINKER_SCRIPT_MEM}\"\nINCLUDE \"${LINKER_SCRIPT_BASE}\""
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    OUTPUT_QUIET
    OUTPUT_FILE ${LINKER_SCRIPT_FILENAME}
  )
else()
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E echo "INCLUDE \"${LINKER_SCRIPT_LIB}\"\nINCLUDE \"${LINKER_SCRIPT_MEM}\"\nINCLUDE \"${LINKER_SCRIPT_BASE}\""
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}}
    OUTPUT_QUIET
    OUTPUT_FILE ${LINKER_SCRIPT_FILENAME}
  )
endif()

# Set specs argument based on specified C/C++ library and language
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
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=nano.specs")
  add_definitions(-D__NEWLIB__ -specs=nano.specs)
# redlib
elseif(${CLIB} STREQUAL redlib)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=redlib.specs")
  add_definitions(-D__REDLIB__ -specs=redlib.specs)
endif()

set(LINKER_SCRIPT ${PROJECT_BINARY_DIR}/${LINKER_SCRIPT_FILENAME} CACHE INTERNAL "Linker script")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -Xlinker --gc-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -T ${LINKER_SCRIPT} -L ${LINKER_SCRIPT_DIR}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(OUTPUT_NAME ${CMAKE_PROJECT_NAME}.axf)
set(FULL_OUTPUT_NAME ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME})