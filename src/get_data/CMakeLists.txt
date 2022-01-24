file(GLOB_RECURSE   GET_DATA_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/*c)
add_library(linux_data ${GET_DATA_SRC})
target_link_libraries(linux_data   pthread )