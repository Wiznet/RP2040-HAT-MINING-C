# ioLibrary_Driver
add_library(IOLIBRARY_FILES STATIC)

target_sources(IOLIBRARY_FILES PUBLIC
        ${PORT_DIR}/ioLibrary_Driver/src/w5x00_spi.c
        ${PORT_DIR}/ioLibrary_Driver/src/w5x00_gpio_irq.c
        )

target_include_directories(IOLIBRARY_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}/ioLibrary_Driver/inc
        ${PORT_DIR}
        )

target_link_libraries(IOLIBRARY_FILES PRIVATE
        ETHERNET_FILES
        AWS_SDK_PORT_FILES 
        TIMER_FILES       
        pico_stdlib
        pico_unique_id
        pico_multicore
        hardware_i2c
        hardware_spi
        hardware_dma
        hardware_clocks
        mbedcrypto
        mbedx509
        mbedtls
        lwip
        UTILS_FILE
        )

# lwip
add_library(LWIP_FILES STATIC)

target_sources(LWIP_FILES PUBLIC
        ${PORT_DIR}/lwip/src/w5x00_lwip.c
        ${PORT_DIR}/lwip/src/tcp_client.c
        )

target_include_directories(LWIP_FILES PUBLIC
        ${PICO_EXTRAS_LWIP_SRC_DIR}
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}
        ${PORT_DIR}/lwip/inc
        )

target_link_libraries(LWIP_FILES PRIVATE
        ETHERNET_FILES
        lwip
        )

# timer
add_library(TIMER_FILES STATIC)

target_sources(TIMER_FILES PUBLIC
        ${PORT_DIR}/timer/timer.c
        )

target_include_directories(TIMER_FILES PUBLIC
        ${PORT_DIR}/timer
        )

target_link_libraries(TIMER_FILES PRIVATE
        pico_stdlib      
        )

# aws-iot-device-sdk-embedded-C
add_library(AWS_SDK_PORT_FILES STATIC)

target_sources(AWS_SDK_PORT_FILES PUBLIC
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/dns_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/http_transport_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/mqtt_transport_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/ssl_transport_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/timer_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/util.c
        )

target_include_directories(AWS_SDK_PORT_FILES PUBLIC
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/inc
        ${MBEDTLS_DIR}/include
        )

target_link_libraries(AWS_SDK_PORT_FILES PUBLIC
        pico_stdlib
        ETHERNET_FILES
        DHCP_FILES
        DNS_FILES
        AWS_SDK_FILES
        mbedcrypto
        mbedx509
        mbedtls
        )

# I2C_SLAVE
add_library(I2C_SLAVE STATIC)

target_sources(I2C_SLAVE PUBLIC
        ${PORT_DIR}/i2c_slave/i2c_slave.c
        )

target_include_directories(I2C_SLAVE PUBLIC
        ${PORT_DIR}
        ${PORT_DIR}/i2c_slave/inc
        ${PORT_DIR}/utils/inc

        )
target_link_libraries(I2C_SLAVE PUBLIC
        UTILS_FILE
        pico_unique_id
        hardware_resets
        hardware_i2c
        hardware_irq
        hardware_clocks
        )

# UTILS_FILE
add_library(UTILS_FILE STATIC)

target_sources(UTILS_FILE PUBLIC
        ${PORT_DIR}/utils/src/charbuffer.cpp
        ${PORT_DIR}/utils/src/duinoCoinUtils.cpp
        ${PORT_DIR}/utils/src/hwSetupUtils.c
        ${PORT_DIR}/utils/src/Wire.cpp
        )

target_include_directories(UTILS_FILE PUBLIC
        ${PORT_DIR}
        ${CMAKE_CURRENT_DIR}
        ${PORT_DIR}/timer
        ${PORT_DIR}/utils/inc
        ${PORT_DIR}/i2c_slave/inc
        )
target_link_libraries(UTILS_FILE PUBLIC
        I2C_SLAVE
        TIMER_FILES
        IOLIBRARY_FILES
        AWS_SDK_PORT_FILES
        ETHERNET_FILES
        LWIP_FILES
        DHCP_FILES
        DNS_FILES        
        pico_unique_id        
        pico_stdlib
        pico_stdio
        hardware_dma
        hardware_i2c
        hardware_irq
        hardware_spi
        mbedx509
        mbedtls
        mbedcrypto
        lwip
        )        