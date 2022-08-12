set(TARGET_NAME duino_coin)

add_executable(${TARGET_NAME}
        ${JSON_SOURCES}
        ${TARGET_NAME}.c
)

target_include_directories(${TARGET_NAME} PUBLIC
        ${JSON_INCLUDE_PUBLIC_DIRS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        pico_multicore
        hardware_spi
        hardware_dma
        ETHERNET_FILES
        IOLIBRARY_FILES
        DHCP_FILES
        DNS_FILES
        AWS_SDK_PORT_FILES
        TIMER_FILES
        mbedcrypto
        mbedx509
        mbedtls
)

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})
