set(TARGET_NAME miner_W5100S-EVB-Pico)

add_executable(${TARGET_NAME}
        ${JSON_SOURCES}
        ${TARGET_NAME}.c
)

target_include_directories(${TARGET_NAME} PUBLIC
        ${JSON_INCLUDE_PUBLIC_DIRS}
        ${PICO_EXTRAS_LWIP_SRC_DIR}
        ${PORT_DIR}/lwip
        ${PORT_DIR}/utils/inc
)

target_link_libraries(${TARGET_NAME} PRIVATE
        ETHERNET_FILES
        HTTPSERVER_FILES
        IOLIBRARY_FILES
        AWS_SDK_PORT_FILES        
        LWIP_FILES
        DHCP_FILES
        DNS_FILES
        TIMER_FILES
        UTILS_FILE
        I2C_SLAVE
        pico_stdlib
        pico_unique_id
        pico_multicore
        hardware_spi
        hardware_dma
        hardware_i2c
        hardware_irq
        lwip
        mbedcrypto
        mbedx509
        mbedtls
)

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})
