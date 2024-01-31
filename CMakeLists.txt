cmake_minimum_required(VERSION 3.9)
project(fast_containers)

set(CMAKE_CXX_STANDARD 20)

include(conanbuildinfo.cmake)
conan_basic_setup()

set(ALLOCATORS_DIRECTORY ${CMAKE_SOURCE_DIR}/allocators/)
set(CONTAINERS_DIRECTORY ${CMAKE_SOURCE_DIR}/containers/)
set(UTILS_DIRECTORY ${CMAKE_SOURCE_DIR}/utils/)

list(APPEND CONTAINERS_DIRECTORIES ${CONTAINERS_DIRECTORY} ${ALLOCATORS_DIRECTORY} ${UTILS_DIRECTORY})

add_subdirectory(example/)
add_subdirectory(benchmarks/)
