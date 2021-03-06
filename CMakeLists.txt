cmake_minimum_required(VERSION 3.0.0)
project(dphpc-project VERSION 0.1.0)

cmake_policy(SET CMP0074 NEW)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mavx2 -mfma")


include(CTest)
enable_testing()

#add_executable(dphpc-project main.cpp)

message("### build mode ### : ${CMAKE_BUILD_TYPE}")

set(
    SRC 
    "jacobi/jacobi1d.cpp"
    "jacobi/jacobi2d.cpp"
    "jacobi/jacobi2d_omp.cpp"
    "doitgen/doitgen.cpp"
    "polybench.cpp"
)

find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)


add_library(dphpc_lib STATIC ${SRC})
message(STATUS "OpenMPI include dir : ${MPI_CXX_INCLUDE_DIRS}")
target_include_directories(dphpc_lib PUBLIC ${MPI_CXX_INCLUDE_DIRS})
target_link_libraries(dphpc_lib PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(dphpc_lib PUBLIC MPI::MPI_CXX)
target_link_libraries(dphpc_lib PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/liblsb/.libs/liblsb.a)

target_include_directories(dphpc_lib
    PUBLIC "jacobi"
    PUBLIC "doitgen"
    PUBLIC "polybench/utilities"
)

add_subdirectory(utils)
add_subdirectory(doitgen/test)
add_subdirectory(doitgen/benchmark)
add_subdirectory(jacobi/test)
add_subdirectory(jacobi/benchmark)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
