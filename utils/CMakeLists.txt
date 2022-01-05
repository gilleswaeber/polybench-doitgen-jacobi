set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mavx2 -mfma -g")
set(
    SRC 
    "serializer.cpp"
    "utils.cpp"
)

add_library(dphpc_lib_utils STATIC ${SRC})

target_include_directories(dphpc_lib_utils
    PUBLIC "."
)