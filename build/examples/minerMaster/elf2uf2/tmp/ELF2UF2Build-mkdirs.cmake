# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/tools/elf2uf2"
  "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/elf2uf2"
  "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/examples/minerMaster/elf2uf2"
  "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/examples/minerMaster/elf2uf2/tmp"
  "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/examples/minerMaster/elf2uf2/src/ELF2UF2Build-stamp"
  "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/examples/minerMaster/elf2uf2/src"
  "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/examples/minerMaster/elf2uf2/src/ELF2UF2Build-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/examples/minerMaster/elf2uf2/src/ELF2UF2Build-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/examples/minerMaster/elf2uf2/src/ELF2UF2Build-stamp${cfgdir}") # cfgdir has leading slash
endif()
