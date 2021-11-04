cmake_minimum_required(VERSION 3.0.0)
project(dphpc-project VERSION 0.1.0)

include(CTest)
enable_testing()

#add_executable(dphpc-project main.cpp)

set(
    SRC 
    "jacobi_1D/jacobi_1D.cpp"
    "doitgen/doitgen.cpp"
    "polybench/utilities/polybench.c"
)

add_library(dphpc_lib STATIC ${SRC})

target_include_directories(dphpc_lib
    PUBLIC "jacobi_1D"
    PUBLIC "doitgen"
    PUBLIC "polybench/utilities"
)

add_subdirectory(doitgen/test)
add_subdirectory(jacobi_1D/test)
add_subdirectory(jacobi_1D/benchmark)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
