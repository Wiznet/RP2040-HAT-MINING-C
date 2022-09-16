# CMake minimum required version
cmake_minimum_required(VERSION 3.12)

# Find git
find_package(Git)

if(NOT Git_FOUND)
	message(FATAL_ERROR "Could not find 'git' tool for RP2040-HAT-MINING-C patching")
endif()

message("RP2040-HAT-MINING-C patch utils found")

set(RP2040_HAT_MINING_C_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(AWS_IOT_DEVICE_SDK_EMBEDDED_C_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/aws-iot-device-sdk-embedded-C")
set(AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP")
set(AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREMQTT_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreMQTT")
set(AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREJSON_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreJSON")
set(AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_HTTP_PARSER_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/aws-iot-device-sdk-embedded-C/libraries/standard/coreHTTP/source/dependency/3rdparty/http_parser")
set(IOLIBRARY_DRIVER_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/ioLibrary_Driver")
set(MBEDTLS_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/mbedtls")
set(PICO_EXTRAS_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/pico-extras")
set(PICO_EXTRAS_LWIP_SRC_DIR "${RP2040_HAT_LWIP_C_SRC_DIR}/libraries/pico-extras/lib/lwip")
set(PICO_SDK_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/pico-sdk")
set(PICO_SDK_TINYUSB_SRC_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/libraries/pico-sdk/lib/tinyusb")
set(RP2040_HAT_MINING_C_PATCH_DIR "${RP2040_HAT_MINING_C_SRC_DIR}/patches")

# Delete untracked files in aws-iot-device-sdk-embedded-C
if(EXISTS "${AWS_IOT_DEVICE_SDK_EMBEDDED_C_SRC_DIR}/.git")
	message("cleaning aws-iot-device-sdk-embedded-C...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_SRC_DIR} reset --hard)
	message("aws-iot-device-sdk-embedded-C cleaned")
endif()

# Delete untracked files in ioLibrary_Driver
if(EXISTS "${IOLIBRARY_DRIVER_SRC_DIR}/.git")
	message("cleaning ioLibrary_Driver...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${IOLIBRARY_DRIVER_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${IOLIBRARY_DRIVER_SRC_DIR} reset --hard)
	message("ioLibrary_Driver cleaned")
endif()

# Delete untracked files in mbedtls
if(EXISTS "${MBEDTLS_SRC_DIR}/.git")
	message("cleaning mbedtls...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${MBEDTLS_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${MBEDTLS_SRC_DIR} reset --hard)
	message("mbedtls cleaned")
endif()

# Delete untracked files in pico-extras
if(EXISTS "${PICO_EXTRAS_SRC_DIR}/.git")
	message("cleaning pico-extras...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PICO_EXTRAS_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PICO_EXTRAS_SRC_DIR} reset --hard)
	message("pico-extras cleaned")
endif()

# Delete untracked files in pico-extras lwip
if(EXISTS "${PICO_EXTRAS_LWIP_SRC_DIR}/.git")
	message("cleaning pico-extras lwip...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PICO_EXTRAS_LWIP_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PICO_EXTRAS_LWIP_SRC_DIR} reset --hard)
	message("pico-extras lwip cleaned")
endif()

# Delete untracked files in pico-sdk
if(EXISTS "${PICO_SDK_SRC_DIR}/.git")
	message("cleaning pico-sdk...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PICO_SDK_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PICO_SDK_SRC_DIR} reset --hard)
	message("pico-sdk cleaned")
endif()

execute_process(COMMAND ${GIT_EXECUTABLE} -C ${RP2040_HAT_MINING_C_SRC_DIR} submodule update --init)

# Delete untracked files in coreHTTP
if(EXISTS "${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_SRC_DIR}/.git")
	message("cleaning aws-iot-device-sdk-embedded-C coreHTTP...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_SRC_DIR} reset --hard)
	message("aws-iot-device-sdk-embedded-C coreHTTP cleaned")
endif()

# Delete untracked files in coreMQTT
if(EXISTS "${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREMQTT_SRC_DIR}/.git")
	message("cleaning aws-iot-device-sdk-embedded-C coreMQTT...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREMQTT_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREMQTT_SRC_DIR} reset --hard)
	message("aws-iot-device-sdk-embedded-C coreMQTT cleaned")
endif()

# Delete untracked files in coreJSON
if(EXISTS "${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREJSON_SRC_DIR}/.git")
	message("cleaning aws-iot-device-sdk-embedded-C coreJSON...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREJSON_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREJSON_SRC_DIR} reset --hard)
	message("aws-iot-device-sdk-embedded-C coreJSON cleaned")
endif()

execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_SRC_DIR} submodule update --init)

# Delete untracked files in http_parser
if(EXISTS "${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_HTTP_PARSER_SRC_DIR}/.git")
	message("cleaning aws-iot-device-sdk-embedded-C coreHTTP http_parser...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_HTTP_PARSER_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_HTTP_PARSER_SRC_DIR} reset --hard)
	message("aws-iot-device-sdk-embedded-C coreHTTP http_parser cleaned")
endif()

execute_process(COMMAND ${GIT_EXECUTABLE} -C ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_SRC_DIR} submodule update --init)

# Delete untracked files in tinyusb
if(EXISTS "${PICO_SDK_TINYUSB_SRC_DIR}/.git")
	message("cleaning pico-sdk tinyusb...")
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PICO_SDK_TINYUSB_SRC_DIR} clean -fdx)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PICO_SDK_TINYUSB_SRC_DIR} reset --hard)
	message("pico-sdk tinyusb cleaned")
endif()

execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PICO_SDK_SRC_DIR} submodule update --init)

# coreHTTP patch
message("submodules aws-iot-device-sdk-embedded-C coreHTTP initialised")

file(GLOB AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_PATCHES 
	"${RP2040_HAT_MINING_C_PATCH_DIR}/01_aws_iot_device_sdk_embedded_c_corehttp_network_interface.patch"
	)

foreach(AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_PATCH IN LISTS AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_PATCHES)
	message("Running patch ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_PATCH}")
	execute_process(
		COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_PATCH}
		WORKING_DIRECTORY ${AWS_IOT_DEVICE_SDK_EMBEDDED_C_COREHTTP_SRC_DIR}
	)
endforeach()

# pico-extras patch
message("submodules pico-extras initialised")

file(GLOB PICO_EXTRAS_PATCHES 
	"${RP2040_HAT_MINING_C_PATCH_DIR}/01_pico-extras_lwip_cmakelists.patch" 
	)

foreach(PICO_EXTRAS_PATCH IN LISTS PICO_EXTRAS_PATCHES)
	message("Running patch ${PICO_EXTRAS_PATCH}")
	execute_process(
		COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace ${PICO_EXTRAS_PATCH}
		WORKING_DIRECTORY ${PICO_EXTRAS_SRC_DIR}
	)
endforeach()

# pico-extras lwip patch
message("submodules pico-extras lwip initialised")

file(GLOB PICO_EXTRAS_LWIP_PATCHES 	
	"${RP2040_HAT_MINING_C_PATCH_DIR}/02_pico-extras_lwip_dns.patch"	
	"${RP2040_HAT_MINING_C_PATCH_DIR}/03_pico-extras_lwip_opt.patch"
	)

foreach(PICO_EXTRAS_LWIP_PATCH IN LISTS PICO_EXTRAS_LWIP_PATCHES)
	message("Running patch ${PICO_EXTRAS_LWIP_PATCH}")
	execute_process(
		COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace ${PICO_EXTRAS_LWIP_PATCH}				
		WORKING_DIRECTORY ${PICO_EXTRAS_LWIP_SRC_DIR}
	)	
endforeach()
