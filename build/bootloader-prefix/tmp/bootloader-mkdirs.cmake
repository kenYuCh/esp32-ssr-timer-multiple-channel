# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/yuhsienchang/esp/v5.2.1/esp-idf/components/bootloader/subproject"
  "/Users/yuhsienchang/Downloads/2025-02-14-Gateway-Linux/esp32/ESP32-S3-WROOM-1-N16R8/switch/2025-03-22-sample-1/build/bootloader"
  "/Users/yuhsienchang/Downloads/2025-02-14-Gateway-Linux/esp32/ESP32-S3-WROOM-1-N16R8/switch/2025-03-22-sample-1/build/bootloader-prefix"
  "/Users/yuhsienchang/Downloads/2025-02-14-Gateway-Linux/esp32/ESP32-S3-WROOM-1-N16R8/switch/2025-03-22-sample-1/build/bootloader-prefix/tmp"
  "/Users/yuhsienchang/Downloads/2025-02-14-Gateway-Linux/esp32/ESP32-S3-WROOM-1-N16R8/switch/2025-03-22-sample-1/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/yuhsienchang/Downloads/2025-02-14-Gateway-Linux/esp32/ESP32-S3-WROOM-1-N16R8/switch/2025-03-22-sample-1/build/bootloader-prefix/src"
  "/Users/yuhsienchang/Downloads/2025-02-14-Gateway-Linux/esp32/ESP32-S3-WROOM-1-N16R8/switch/2025-03-22-sample-1/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/yuhsienchang/Downloads/2025-02-14-Gateway-Linux/esp32/ESP32-S3-WROOM-1-N16R8/switch/2025-03-22-sample-1/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/yuhsienchang/Downloads/2025-02-14-Gateway-Linux/esp32/ESP32-S3-WROOM-1-N16R8/switch/2025-03-22-sample-1/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
