file(GLOB_RECURSE   LV_APP  ${CMAKE_CURRENT_SOURCE_DIR}/*c  ${CMAKE_CURRENT_SOURCE_DIR}/images/*c)
add_definitions(-DLV_LVGL_H_INCLUDE_SIMPLE)

include_directories(${CMAKE_SOURCE_DIR}/src/lvgl/lvgl/ )
include_directories(${CMAKE_SOURCE_DIR}/src/oled_driver/ )
include_directories( ${CMAKE_SOURCE_DIR}/src/lvgl/lvgl/examples/)
include_directories( ${CMAKE_SOURCE_DIR}/src/get_data)
add_executable(lv_app ${LV_APP})
target_link_libraries(lv_app lvgl oled_driver  linux_data )