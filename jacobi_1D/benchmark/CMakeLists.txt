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

target_link_libraries(dphpc-jacobi-benchmarks PUBLIC 
    dphpc_lib 
    check 
    m 
    rt
    pthread 
    ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a
)
# subunit
