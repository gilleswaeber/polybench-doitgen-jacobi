cmake_minimum_required(VERSION 3.0.0)
project(dphpc-jacobi-test VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")


set(
    JACOBI_TEST_SRC
    "main.cpp"
)

target_include_directories(dphpc_lib
        PUBLIC "../../"
        )

add_executable(dphpc-jacobi-test ${JACOBI_TEST_SRC})
add_executable(dphpc-jacobi-mpi-test "jacobi1dMpiTest.cpp")

target_link_libraries(dphpc-jacobi-test PUBLIC dphpc_lib dphpc_lib_utils check m rt pthread)
target_link_libraries(dphpc-jacobi-mpi-test PUBLIC dphpc_lib dphpc_lib_utils check m rt pthread)
