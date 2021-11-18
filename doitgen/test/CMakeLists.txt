cmake_minimum_required(VERSION 3.0.0)
project(dphpc-doitgen-test VERSION 0.1.0)


set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-march=native -ffast-math -mprefer-vector-width=512 -O3")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(
    DOITGEN_TEST_SRC
    #"serializer.cpp"
    "main.cpp"
)

set(
    DOITGEN_DATASET_GEN_SRC
    "generate_datasets.cpp"
)

add_executable(dphpc-doitgen-test ${DOITGEN_TEST_SRC})
add_executable(dphpc-doitgen-gen ${DOITGEN_DATASET_GEN_SRC})

target_link_libraries(dphpc-doitgen-test PUBLIC dphpc_lib_utils dphpc_lib check m rt pthread subunit)
target_link_libraries(dphpc-doitgen-gen PUBLIC dphpc_lib_utils dphpc_lib check m rt pthread subunit)