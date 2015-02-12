add_definitions(
  -DCORE_M4
  -D__USE_LPCOPEN
  -D__NEWLIB__
  -D__CODE_RED

  -Wall                         # Enables all compiler warning messages

  -c                            # Compiles source files without linking

  -fmessage-length=0            # No line-wrapping for error messages (each message appears on a single line)
  -ffunction-sections           # Place each function into its own section in the output file
  -fdata-sections               # Place each data item into its own section in the output file
  -fsingle-precision-constant   # Treat floating point constant as single precision constant instead of implicitly converting it to double precision constant
  -fno-builtin                  # Don't recognize built-in functions that do not begin with `__builtin_' as prefix

  -mcpu=cortex-m4
  -mthumb
  -mfpu=fpv4-sp-d16
  -mfloat-abi=softfp
)

if(DEFINED CMAKE_RELEASE)
  add_definitions(-O2 -Os)          # O2: Optimize even more, Os: Optimize for size
else()
  add_definitions(-O0 -g3 -DDEBUG)  # O0: Reduce compilation time and make debugging produce the expected results, g3: Level 3 debugging information
endif()

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx.ld")

# Set specs argument based on specified C library
if(${CLIB} MATCHES newlib-nano)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=nano.specs -u _printf_float -u _scanf_float")
elseif(${CLIB} MATCHES none)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=nosys.specs")
elseif(${CLIB} MATCHES redlib)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=redlib.specs")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -Xlinker --gc-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -T ${LINKER_SCRIPT}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(OUTPUT_NAME ${CMAKE_PROJECT_NAME}.axf)
set(FULL_OUTPUT_NAME ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME})