cmake_minimum_required(VERSION 3.0.0)
project(dphpc-doitgen-test VERSION 0.1.0)


set(
    TEST_SRC
    "main.cpp"
)

set(ALL_SRC ${TEST_SRC})

add_executable(dphpc-doitgen-test ${ALL_SRC})

target_link_libraries(dphpc-doitgen-test PUBLIC dphpc_lib check m rt pthread subunit)
