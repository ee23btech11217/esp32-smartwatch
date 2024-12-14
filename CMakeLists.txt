cmake_minimum_required(VERSION 3.16)

set(PARTITION_TABLE_CSV "partitions.csv")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/managed_components/esp32-max30100")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(display)
