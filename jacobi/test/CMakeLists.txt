cmake_minimum_required(VERSION 3.0.0)
project(dphpc-jacobi-test VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

target_include_directories(dphpc_lib PUBLIC "../../")

find_library(SUBUNIT_LIB subunit)

add_executable(jacobi1d-check "check1d.cpp")
add_executable(jacobi1d-mpi-test "mpi.cpp")
add_executable(jacobi2d-check "check2d.cpp")
add_executable(jacobi2d-mpi-test "mpi2d.cpp")
add_executable(jacobi2d-mpi-2step-test "mpi2d.cpp")
add_executable(jacobi2d-mpi-vstack-test "mpi2d.cpp")

target_link_libraries(jacobi1d-check PUBLIC dphpc_lib_utils dphpc_lib check m rt pthread)
target_link_libraries(jacobi1d-mpi-test PUBLIC dphpc_lib_utils dphpc_lib dphpc_lib_utils)
target_link_libraries(jacobi2d-check PUBLIC dphpc_lib_utils dphpc_lib check m rt pthread)
target_link_libraries(jacobi2d-mpi-test PUBLIC dphpc_lib_utils dphpc_lib dphpc_lib_utils)
target_link_libraries(jacobi2d-mpi-2step-test PUBLIC dphpc_lib_utils dphpc_lib dphpc_lib_utils)
target_link_libraries(jacobi2d-mpi-vstack-test PUBLIC dphpc_lib_utils dphpc_lib dphpc_lib_utils)

target_compile_definitions(jacobi2d-mpi-2step-test PUBLIC -DJACOBI2D_TWO_STEPS_SYNC)
target_compile_definitions(jacobi2d-mpi-vstack-test PUBLIC -DJACOBI2D_VSTACK)
