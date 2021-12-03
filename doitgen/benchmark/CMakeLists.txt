cmake_minimum_required(VERSION 3.0.0)
project(dphpc-doitgen-benchmark VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(
    DOITGEN_OPENMP_BENCHMARK_SRC
    "openMPBenchmark.cpp"
)

set(
    DOITGEN_OPENMP_BLOCKING_BENCHMARK_SRC
    "openMPBlockingBenchmark.cpp"
)

set(
    DOITGEN_OPENMPI_BENCHMARK_SRC
    "openMPIBenchmark.cpp"
)

set(
    DOITGEN_SEQUENTIAL_BENCHMARK_SRC
    "sequentialBenchmark.cpp"
)

add_executable(dphpc-doitgen-openmp-benchmark ${DOITGEN_OPENMP_BENCHMARK_SRC})
add_executable(dphpc-doitgen-mpi-benchmark ${DOITGEN_OPENMPI_BENCHMARK_SRC})
add_executable(dphpc-doitgen-openmp-blocking-benchmark ${DOITGEN_OPENMP_BLOCKING_BENCHMARK_SRC})

target_link_libraries(dphpc-doitgen-mpi-benchmark PUBLIC dphpc_lib_utils dphpc_lib ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a)
target_link_libraries(dphpc-doitgen-openmp-benchmark PUBLIC dphpc_lib_utils dphpc_lib ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a)
target_link_libraries(dphpc-doitgen-openmp-blocking-benchmark PUBLIC dphpc_lib_utils dphpc_lib ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a)

add_executable(dphpc-doitgen-sequential-benchmark ${DOITGEN_SEQUENTIAL_BENCHMARK_SRC})
target_link_libraries(dphpc-doitgen-sequential-benchmark PUBLIC dphpc_lib_utils dphpc_lib ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a)