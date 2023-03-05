cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(smv)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited config" FORCE)
set(CMAKE_CXX_STANDARD 17)

# enable warnings
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

find_package(MPI REQUIRED)
find_package(OpenMP)

add_executable(smv smv.cpp)
target_include_directories(smv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(smv PRIVATE MPI::MPI_CXX)

if (OpenMP_FOUND)
    target_link_libraries(smv PRIVATE OpenMP::OpenMP_CXX)
endif()
