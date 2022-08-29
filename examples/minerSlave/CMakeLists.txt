set(TARGET_NAME minerSlave)

add_executable(${TARGET_NAME} ${JSON_SOURCES} minerSlave.cpp)

pico_add_extra_outputs(${TARGET_NAME} )

pico_enable_stdio_usb(${TARGET_NAME}  0)
pico_enable_stdio_uart(${TARGET_NAME}  1)

target_include_directories( ${TARGET_NAME} PUBLIC
        ${JSON_INCLUDE_PUBLIC_DIRS}
        ${PICO_EXTRAS_LWIP_SRC_DIR}
        ${PORT_DIR}/lwip
        ${PORT_DIR}/i2c_slave/inc
        ${PORT_DIR}/utils/inc
        ILI9340/
)

target_compile_options(${TARGET_NAME}  PRIVATE -Wall)

target_link_libraries(${TARGET_NAME}  PRIVATE 
UTILS_FILE TIMER_FILES pico_multicore hardware_i2c hardware_irq pico_stdlib mbedcrypto mbedx509 mbedtls)