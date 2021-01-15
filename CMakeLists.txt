cmake_minimum_required(VERSION 3.15)
project(tcg-cdc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -march=native")

add_subdirectory(src)
