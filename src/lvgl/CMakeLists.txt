cmake_minimum_required(VERSION 3.10) 
# add the executable 
file(GLOB_RECURSE   LVGL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/*c)
add_library(lvgl  ${LVGL_SRC})

