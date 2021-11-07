cmake_minimum_required(VERSION 3.0.0)
project(dphpc-doitgen-test VERSION 0.1.0)


set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(
    DOITGEN_TEST_SRC
    "main.cpp"
)

target_include_directories(dphpc_lib
    PUBLIC "../../"
)

add_executable(dphpc-doitgen-test ${DOITGEN_TEST_SRC})

target_link_libraries(dphpc-doitgen-test PUBLIC dphpc_lib check m rt pthread subunit)
