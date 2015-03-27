add_custom_target(hex
  DEPENDS ${FULL_OUTPUT_NAME}
  COMMAND ${CMAKE_OBJCOPY} -v -Oihex ${EXECUTABLE_OUTPUT_PATH}/${FULL_OUTPUT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME}.hex
)

if(${HAVE_CHECKSUM_TOOL})
  add_custom_target(bin
    DEPENDS ${FULL_OUTPUT_NAME}
    COMMAND ${CMAKE_OBJCOPY} -v -Obinary ${EXECUTABLE_OUTPUT_PATH}/${FULL_OUTPUT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME}.bin
    COMMAND ${CHECKSUM_TOOL} -p ${DEVICE} -d ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME}.bin
  )
else()
  add_custom_target(bin
    DEPENDS ${FULL_OUTPUT_NAME}
    COMMAND ${CMAKE_OBJCOPY} -v -Obinary ${EXECUTABLE_OUTPUT_PATH}/${FULL_OUTPUT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME}.bin
  )
endif()

add_custom_target(lst
  DEPENDS ${FULL_OUTPUT_NAME}
  COMMAND ${CMAKE_OBJDUMP} -x -D ${EXECUTABLE_OUTPUT_PATH}/${FULL_OUTPUT_NAME} > ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME}.lst
)

if(${HAVE_OPENOCD})
  add_custom_target(flash
    DEPENDS ${FULL_OUTPUT_NAME}
    COMMAND make bin
    COMMAND ${OPENOCD_BINARY} -c "set _TRANSPORT ${OPENOCD_TRANSPORT}" -f ${CMAKE_SOURCE_DIR}/debug/${OPENOCD_CONFIG} -c "init; reset; sleep 500; halt; flash protect ${FLASH_BANK} 0 last off; flash write_image erase ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME}.bin ${FLASH_BANK_ADDRESS} bin; sleep 500; reset; shutdown;"
  )
  add_custom_target(erase
    DEPENDS ${FULL_OUTPUT_NAME}
    COMMAND ${OPENOCD_BINARY} -c "set _TRANSPORT ${OPENOCD_TRANSPORT}" -f ${CMAKE_SOURCE_DIR}/debug/${OPENOCD_CONFIG} -c "init; reset; sleep 500; halt; flash protect ${FLASH_BANK} 0 last off; flash erase_sector ${FLASH_BANK} 0 last; sleep 500; reset; shutdown;"
  )
endif()