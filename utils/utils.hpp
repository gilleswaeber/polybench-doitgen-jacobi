#pragma once

#include <stdlib.h>
#include <string>
#include <iostream> 

#define POLYBENCH_CACHE_SIZE_KB 32770



extern void* allocate_data(unsigned long long int elem_n, int size_of_type);
extern void cleanup(void* ptr);
extern void flush_cache();
extern void flush_cache_openMP();
extern void flush_cache_big();

extern std::string print_array3D(double* arr, uint64_t nr, uint64_t nq, uint64_t np);
extern std::string print_array2D(double* arr, uint64_t nq, uint64_t np);