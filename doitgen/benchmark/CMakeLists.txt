cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
project(dphpc-doitgen-benchmark VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-march=native -ffast-math -mprefer-vector-width=512 -O3")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(
    DOITGEN_OPENMP_BENCHMARK_SRC
    "openMPBenchmark.cpp"
)

set(
    DOITGEN_OPENMPI_BENCHMARK_SRC
    "openMPIBenchmark.cpp"
)

add_executable(dphpc-doitgen-openmp-benchmark ${DOITGEN_OPENMP_BENCHMARK_SRC})
add_executable(dphpc-doitgen-mpi-benchmark ${DOITGEN_OPENMPI_BENCHMARK_SRC})

target_link_libraries(dphpc-doitgen-mpi-benchmark PUBLIC dphpc_lib_utils dphpc_lib ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a papi)
target_link_libraries(dphpc-doitgen-openmp-benchmark PUBLIC dphpc_lib_utils dphpc_lib ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a papi)