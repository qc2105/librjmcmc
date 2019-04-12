cmake_minimum_required(VERSION 2.8)
project(librjmcmc CXX)

set(CMAKE_BUILD_TYPE "Release")
INCLUDE_DIRECTORIES(extern)
add_subdirectory(src)
add_subdirectory(samples)
add_subdirectory(doc)
