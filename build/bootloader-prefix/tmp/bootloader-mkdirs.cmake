# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/mnepraj/esp/esp-idf/components/bootloader/subproject"
  "/home/mnepraj/display/build/bootloader"
  "/home/mnepraj/display/build/bootloader-prefix"
  "/home/mnepraj/display/build/bootloader-prefix/tmp"
  "/home/mnepraj/display/build/bootloader-prefix/src/bootloader-stamp"
  "/home/mnepraj/display/build/bootloader-prefix/src"
  "/home/mnepraj/display/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/mnepraj/display/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/mnepraj/display/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
