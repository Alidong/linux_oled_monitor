cmake_minimum_required(VERSION 3.10) 
message(STATUS $ENV{CXXFLAGS})
set( CMAKE_BUILD_TYPE "Debug" )
SET(CMAKE_C_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb -std=gnu11")
SET(CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# set the project name 
project(Oled_monitor) 
add_subdirectory(./src/lvgl)
add_subdirectory(./src/oled_driver)
add_subdirectory(./src/lv_app)
add_subdirectory(./src/get_data)