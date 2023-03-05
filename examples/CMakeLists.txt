cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(Parallel-Programming-Examples)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited config" FORCE)
set(CMAKE_CXX_STANDARD 17)

# enable warnings
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(dmv)
add_subdirectory(fib)
add_subdirectory(hello)
add_subdirectory(all_pairs)
add_subdirectory(cond_vars)
add_subdirectory(x86)
