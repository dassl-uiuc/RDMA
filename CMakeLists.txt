cmake_minimum_required(VERSION 3.14)

project(infinity)
set(CMAKE_CXX_STANDARD 14)
set(RDMA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message("${RDMA_SOURCE_DIR}")

find_package(Threads REQUIRED)

add_subdirectory(src)
