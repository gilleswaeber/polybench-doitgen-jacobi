cmake_minimum_required(VERSION 3.0.0)
project(dphpc-jacobi-benchmarks VERSION 0.1.0)

target_include_directories(dphpc_lib PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/liblsb")

add_executable(dphpc-jacobi-mpi-benchmark mpi_benchmark.cpp)
add_executable(dphpc-jacobi-benchmark main.cpp)

target_link_libraries(dphpc-jacobi-benchmark PUBLIC dphpc_lib dphpc_lib_utils ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a)
target_link_libraries(dphpc-jacobi-mpi-benchmark PUBLIC dphpc_lib dphpc_lib_utils ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a)
