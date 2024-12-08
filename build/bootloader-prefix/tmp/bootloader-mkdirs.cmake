# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/mnepraj/esp/esp-idf/components/bootloader/subproject"
  "/home/mnepraj/smart-watch/build/bootloader"
  "/home/mnepraj/smart-watch/build/bootloader-prefix"
  "/home/mnepraj/smart-watch/build/bootloader-prefix/tmp"
  "/home/mnepraj/smart-watch/build/bootloader-prefix/src/bootloader-stamp"
  "/home/mnepraj/smart-watch/build/bootloader-prefix/src"
  "/home/mnepraj/smart-watch/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/mnepraj/smart-watch/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/mnepraj/smart-watch/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
