cmake_minimum_required(VERSION 3.0.0)
project(dphpc-jacobi-benchmarks VERSION 0.1.0)


set(
    TEST_SRC
    "main.cpp"
)

set(ALL_SRC ${TEST_SRC})

target_include_directories(dphpc_lib
    PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/liblsb"
)

add_executable(dphpc-jacobi-benchmarks ${ALL_SRC})

# credits to https://stackoverflow.com/questions/23163075/how-to-compile-an-mpi-included-c-program-using-cmake
# OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(dphpc-jacobi-benchmarks PUBLIC OpenMP::OpenMP_CXX)
endif()

# MPI
find_package(MPI REQUIRED)
target_link_libraries(dphpc-jacobi-benchmarks PUBLIC MPI::MPI_CXX)

target_link_libraries(dphpc-jacobi-benchmarks PUBLIC 
    dphpc_lib 
    check 
    m 
    rt
    pthread 
    subunit
    ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a
    
)