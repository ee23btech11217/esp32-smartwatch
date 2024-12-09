cmake_minimum_required(VERSION 3.16)

set(PARTITION_TABLE_CSV "partitions.csv")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/components/wifi_manager
    ${CMAKE_CURRENT_LIST_DIR}/components/lvgl
    ${CMAKE_CURRENT_LIST_DIR}/components/lvgl_esp32_drivers
)


project(Grand-challenge)