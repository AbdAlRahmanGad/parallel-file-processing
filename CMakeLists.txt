cmake_minimum_required(VERSION 3.28)
project(parallel_file_processing C)
find_package(MPI REQUIRED)

set(CMAKE_C_STANDARD 11)

add_executable(parallel_file_processing main.c)

target_link_libraries(parallel_file_processing MPI::MPI_C)
