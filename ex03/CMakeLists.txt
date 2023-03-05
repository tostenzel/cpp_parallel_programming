cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(matadd)
include_directories (include)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited config" FORCE)
set(CMAKE_CXX_STANDARD 17)

# enable warnings
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -pedantic -lpthread)
endif()

find_package (Threads)

file(GLOB SOURCES "src/*.cpp")
add_executable(matadd ${SOURCES})
target_link_libraries (matadd ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(matadd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
