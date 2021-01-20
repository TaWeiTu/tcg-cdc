cmake_minimum_required(VERSION 3.15)
project(tcg-cdc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")


# Enable cppcheck, clang-tidy and cpplint
set(CMAKE_CXX_CPPLINT "cpplint")
# set(CMAKE_CXX_CPPCHECK "cppcheck")
# set(CMAKE_CXX_CLANG_TIDY 
#     "clang-tidy;--checks=-*,cppcoreguidelines-*,clang-analyzer-*,google-*performance-*,portability-*,readability-*")

include_directories(include)
add_subdirectory(src)
