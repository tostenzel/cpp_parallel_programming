cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(blur)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited config" FORCE)
set(CMAKE_CXX_STANDARD 17)

# enable warnings
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

find_package(OpenCL REQUIRED)

add_executable(blur blur.cpp)
target_include_directories(blur PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(blur OpenCL::OpenCL)
