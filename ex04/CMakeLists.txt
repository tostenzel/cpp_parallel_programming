cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(mandelbrot)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited config" FORCE)
set(CMAKE_CXX_STANDARD 17)

# enable warnings
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_compile_options(-mavx -O3)

add_executable(mandelbrot mandelbrot.cpp)
target_include_directories(mandelbrot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
