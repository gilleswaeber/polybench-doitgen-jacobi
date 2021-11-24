set(CMAKE_CXX_FLAGS_RELEASE "-march=native -ffast-math -O3")

set(
    SRC 
    "serializer.cpp"
    "utils.cpp"
)

add_library(dphpc_lib_utils STATIC ${SRC})

target_include_directories(dphpc_lib_utils
    PUBLIC "."
)