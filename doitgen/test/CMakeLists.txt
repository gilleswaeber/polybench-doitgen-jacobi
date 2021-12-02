cmake_minimum_required(VERSION 3.0.0)
project(dphpc-doitgen-test VERSION 0.1.0)

add_definitions(-DTEST_DIRECTORY_PATH="${CMAKE_CURRENT_BINARY_DIR}")

message("doitgen test binary directory at ${CMAKE_CURRENT_BINARY_DIR}")
message("doitgen test directory at ${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-march=native -ffast-math -O3")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(
    DOITGEN_TEST_SRC
    "openMPTest.cpp"
)

set(
    DOITGEN_DATASET_GEN_SRC
    "generate_datasets.cpp"
)

find_library(SUBUNIT_LIB subunit)

if(SUBUNIT_LIB)
    message("subunit found !")
    
    add_executable(dphpc-doitgen-test ${DOITGEN_TEST_SRC})
    add_executable(dphpc-doitgen-mpi-test "openMPITest.cpp")
    add_executable(dphpc-doitgen-gen ${DOITGEN_DATASET_GEN_SRC})

    target_link_libraries(dphpc-doitgen-test PUBLIC dphpc_lib_utils dphpc_lib check m rt pthread subunit)
    target_link_libraries(dphpc-doitgen-mpi-test PUBLIC dphpc_lib_utils dphpc_lib check m rt pthread subunit)
    target_link_libraries(dphpc-doitgen-gen PUBLIC dphpc_lib_utils dphpc_lib check m rt pthread subunit)
endif()