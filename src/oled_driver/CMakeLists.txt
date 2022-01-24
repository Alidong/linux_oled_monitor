cmake_minimum_required(VERSION 3.10) 
# add the executable 
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE   OLED_DRIVER  ${CMAKE_CURRENT_SOURCE_DIR}/*c)
add_library(oled_driver  ${OLED_DRIVER})
target_link_libraries(oled_driver  lvgl)
