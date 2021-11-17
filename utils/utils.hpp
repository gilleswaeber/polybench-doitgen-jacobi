#pragma once

# include <stdlib.h>

#define POLYBENCH_CACHE_SIZE_KB 32770

extern void* allocate_data(unsigned long long int elem_n, int size_of_type);
extern void cleanup(void* ptr);
extern void flush_cache();
