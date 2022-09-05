# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.24

# compile ASM with /Applications/ARM/bin/arm-none-eabi-gcc
# compile C with /Applications/ARM/bin/arm-none-eabi-gcc
# compile CXX with /Applications/ARM/bin/arm-none-eabi-g++
ASM_DEFINES = -DLIB_PICO_BIT_OPS=1 -DLIB_PICO_BIT_OPS_PICO=1 -DLIB_PICO_DIVIDER=1 -DLIB_PICO_DIVIDER_HARDWARE=1 -DLIB_PICO_DOUBLE=1 -DLIB_PICO_DOUBLE_PICO=1 -DLIB_PICO_FLOAT=1 -DLIB_PICO_FLOAT_PICO=1 -DLIB_PICO_INT64_OPS=1 -DLIB_PICO_INT64_OPS_PICO=1 -DLIB_PICO_MALLOC=1 -DLIB_PICO_MEM_OPS=1 -DLIB_PICO_MEM_OPS_PICO=1 -DLIB_PICO_PLATFORM=1 -DLIB_PICO_PRINTF=1 -DLIB_PICO_PRINTF_PICO=1 -DLIB_PICO_RUNTIME=1 -DLIB_PICO_STANDARD_LINK=1 -DLIB_PICO_STDIO=1 -DLIB_PICO_STDIO_UART=1 -DLIB_PICO_STDLIB=1 -DLIB_PICO_SYNC=1 -DLIB_PICO_SYNC_CORE=1 -DLIB_PICO_SYNC_CRITICAL_SECTION=1 -DLIB_PICO_SYNC_MUTEX=1 -DLIB_PICO_SYNC_SEM=1 -DLIB_PICO_TIME=1 -DLIB_PICO_UTIL=1 -DMBEDTLS_CONFIG_FILE=\"/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/port/mbedtls/inc/ssl_config.h\" -DPICO_BOARD=\"pico\" -DPICO_BUILD=1 -DPICO_CMAKE_BUILD_TYPE=\"Release\" -DPICO_COPY_TO_RAM=0 -DPICO_CXX_ENABLE_EXCEPTIONS=0 -DPICO_EXTRAS=1 -DPICO_NO_FLASH=0 -DPICO_NO_HARDWARE=0 -DPICO_ON_DEVICE=1 -DPICO_USE_BLOCKED_RAM=0 -DSET_TRUSTED_CERT_IN_SAMPLES -D_WIZCHIP_=W5100S

ASM_INCLUDES = -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/port/aws-iot-device-sdk-embedded-C/inc -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_stdlib/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_gpio/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_base/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/generated/pico_base -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/boards/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_platform/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2040/hardware_regs/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_base/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2040/hardware_structs/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_claim/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_sync/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_uart/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_divider/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_time/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_timer/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_sync/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_util/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_runtime/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_clocks/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_irq/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_resets/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_pll/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_vreg/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_watchdog/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_xosc/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_printf/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_bootrom/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_bit_ops/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_divider/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_double/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_int64_ops/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_float/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_malloc/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/boot_stage2/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_binary_info/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_stdio/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_stdio_uart/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Ethernet -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Ethernet/W5100S -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Internet/DHCP -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Internet/DNS -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/interface -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/dependency/3rdparty/http_parser -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreMQTT/source/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreMQTT/source/interface -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreJSON/source/include

ASM_FLAGS = -mcpu=cortex-m0plus -mthumb -O3 -DNDEBUG -ffunction-sections -fdata-sections

C_DEFINES = -DLIB_PICO_BIT_OPS=1 -DLIB_PICO_BIT_OPS_PICO=1 -DLIB_PICO_DIVIDER=1 -DLIB_PICO_DIVIDER_HARDWARE=1 -DLIB_PICO_DOUBLE=1 -DLIB_PICO_DOUBLE_PICO=1 -DLIB_PICO_FLOAT=1 -DLIB_PICO_FLOAT_PICO=1 -DLIB_PICO_INT64_OPS=1 -DLIB_PICO_INT64_OPS_PICO=1 -DLIB_PICO_MALLOC=1 -DLIB_PICO_MEM_OPS=1 -DLIB_PICO_MEM_OPS_PICO=1 -DLIB_PICO_PLATFORM=1 -DLIB_PICO_PRINTF=1 -DLIB_PICO_PRINTF_PICO=1 -DLIB_PICO_RUNTIME=1 -DLIB_PICO_STANDARD_LINK=1 -DLIB_PICO_STDIO=1 -DLIB_PICO_STDIO_UART=1 -DLIB_PICO_STDLIB=1 -DLIB_PICO_SYNC=1 -DLIB_PICO_SYNC_CORE=1 -DLIB_PICO_SYNC_CRITICAL_SECTION=1 -DLIB_PICO_SYNC_MUTEX=1 -DLIB_PICO_SYNC_SEM=1 -DLIB_PICO_TIME=1 -DLIB_PICO_UTIL=1 -DMBEDTLS_CONFIG_FILE=\"/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/port/mbedtls/inc/ssl_config.h\" -DPICO_BOARD=\"pico\" -DPICO_BUILD=1 -DPICO_CMAKE_BUILD_TYPE=\"Release\" -DPICO_COPY_TO_RAM=0 -DPICO_CXX_ENABLE_EXCEPTIONS=0 -DPICO_EXTRAS=1 -DPICO_NO_FLASH=0 -DPICO_NO_HARDWARE=0 -DPICO_ON_DEVICE=1 -DPICO_USE_BLOCKED_RAM=0 -DSET_TRUSTED_CERT_IN_SAMPLES -D_WIZCHIP_=W5100S

C_INCLUDES = -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/port/aws-iot-device-sdk-embedded-C/inc -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_stdlib/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_gpio/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_base/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/generated/pico_base -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/boards/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_platform/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2040/hardware_regs/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_base/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2040/hardware_structs/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_claim/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_sync/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_uart/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_divider/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_time/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_timer/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_sync/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_util/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_runtime/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_clocks/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_irq/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_resets/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_pll/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_vreg/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_watchdog/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_xosc/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_printf/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_bootrom/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_bit_ops/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_divider/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_double/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_int64_ops/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_float/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_malloc/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/boot_stage2/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_binary_info/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_stdio/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_stdio_uart/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Ethernet -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Ethernet/W5100S -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Internet/DHCP -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Internet/DNS -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/interface -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/dependency/3rdparty/http_parser -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreMQTT/source/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreMQTT/source/interface -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreJSON/source/include

C_FLAGS = -mcpu=cortex-m0plus -mthumb -O3 -DNDEBUG -ffunction-sections -fdata-sections -std=gnu11

CXX_DEFINES = -DLIB_PICO_BIT_OPS=1 -DLIB_PICO_BIT_OPS_PICO=1 -DLIB_PICO_DIVIDER=1 -DLIB_PICO_DIVIDER_HARDWARE=1 -DLIB_PICO_DOUBLE=1 -DLIB_PICO_DOUBLE_PICO=1 -DLIB_PICO_FLOAT=1 -DLIB_PICO_FLOAT_PICO=1 -DLIB_PICO_INT64_OPS=1 -DLIB_PICO_INT64_OPS_PICO=1 -DLIB_PICO_MALLOC=1 -DLIB_PICO_MEM_OPS=1 -DLIB_PICO_MEM_OPS_PICO=1 -DLIB_PICO_PLATFORM=1 -DLIB_PICO_PRINTF=1 -DLIB_PICO_PRINTF_PICO=1 -DLIB_PICO_RUNTIME=1 -DLIB_PICO_STANDARD_LINK=1 -DLIB_PICO_STDIO=1 -DLIB_PICO_STDIO_UART=1 -DLIB_PICO_STDLIB=1 -DLIB_PICO_SYNC=1 -DLIB_PICO_SYNC_CORE=1 -DLIB_PICO_SYNC_CRITICAL_SECTION=1 -DLIB_PICO_SYNC_MUTEX=1 -DLIB_PICO_SYNC_SEM=1 -DLIB_PICO_TIME=1 -DLIB_PICO_UTIL=1 -DMBEDTLS_CONFIG_FILE=\"/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/port/mbedtls/inc/ssl_config.h\" -DPICO_BOARD=\"pico\" -DPICO_BUILD=1 -DPICO_CMAKE_BUILD_TYPE=\"Release\" -DPICO_COPY_TO_RAM=0 -DPICO_CXX_ENABLE_EXCEPTIONS=0 -DPICO_EXTRAS=1 -DPICO_NO_FLASH=0 -DPICO_NO_HARDWARE=0 -DPICO_ON_DEVICE=1 -DPICO_USE_BLOCKED_RAM=0 -DSET_TRUSTED_CERT_IN_SAMPLES -D_WIZCHIP_=W5100S

CXX_INCLUDES = -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/port/aws-iot-device-sdk-embedded-C/inc -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_stdlib/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_gpio/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_base/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/generated/pico_base -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/boards/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_platform/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2040/hardware_regs/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_base/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2040/hardware_structs/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_claim/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_sync/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_uart/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_divider/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_time/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_timer/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_sync/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_util/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_runtime/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_clocks/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_irq/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_resets/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_pll/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_vreg/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_watchdog/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/hardware_xosc/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_printf/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_bootrom/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_bit_ops/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_divider/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_double/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_int64_ops/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_float/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_malloc/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/boot_stage2/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/common/pico_binary_info/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_stdio/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/src/rp2_common/pico_stdio_uart/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Ethernet -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Ethernet/W5100S -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Internet/DHCP -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/ioLibrary_Driver/Internet/DNS -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/interface -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/dependency/3rdparty/http_parser -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreMQTT/source/include -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreMQTT/source/interface -I/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreJSON/source/include

CXX_FLAGS = -mcpu=cortex-m0plus -mthumb -O3 -DNDEBUG -ffunction-sections -fdata-sections -fno-exceptions -fno-unwind-tables -fno-rtti -fno-use-cxa-atexit -std=gnu++17
