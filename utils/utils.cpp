/**
 * Taken from polybench by Louis
 * 
 */

#include "utils.hpp"

#include <string.h>
#include <iostream>
#include <assert.h>
#include <cstdio>

static
void *
xmalloc (size_t num)
{
  void* new_cpp = NULL;
  int ret = posix_memalign (&new_cpp, 32, num);
  if (!new_cpp || ret)
    {
      fprintf (stderr, "[PolyBench] posix_memalign: cannot allocate memory");
      exit (1);
    }
  return new_cpp;
}

/**
 * @brief copied from polybench
 * 
 * @param n 
 * @param elt_size 
 * @return void* 
 */
void* allocate_data(unsigned long long int n, int elt_size)
{
  /// FIXME: detect overflow!
  size_t val = n;
  val *= elt_size;
  void* ret = xmalloc(val);

  return ret;
}

void flush_cache()
{
  unsigned long long cs = POLYBENCH_CACHE_SIZE_KB * 1024ULL / (unsigned long long)sizeof(double);
  double* flush = (double*) calloc (cs, sizeof(double));
  unsigned long long i;
  double tmp = 0.0;
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (i = 0; i < cs; i++)
    tmp += flush[i];
  //This is to prevent compiler optimizations
  if (tmp > 10.0) {
	  std::cout << "Fail !" << std::endl;
  }
  assert (tmp <= 10.0);
  free ((void*)flush);
}

extern void cleanup(void* ptr) {
  free((void*)ptr);
}

#define ARR_2D_UTILS(ARRAY, Y_DIM, X, Y) (ARRAY[ (X) * (Y_DIM) + (Y) ])

#define ARR_3D_UTILS(ARRAY, X_DIM, Y_DIM, Z_DIM, X, Y, Z) \
	(ARRAY[ ((Z_DIM) * (Y_DIM) * (X)) + ((Z_DIM) * (Y)) + (Z) ])

extern std::string print_array3D(double* arr, uint64_t nr, uint64_t nq, uint64_t np) {
	std::string result = "";
	result += "################### 3D array bellow ######################\n";
	for (uint64_t i = 0; i < nr; i++) {
		for (uint64_t j = 0; j < nq; j++) {
			for (uint64_t k = 0; k < np; k++) {
				result += std::to_string(ARR_3D_UTILS(arr, nr, nq, np, i, j, k)) + " ";
			}
			result += "\n";
		}
		result += "-------------------------------------\n";
	}
	return result;
}

extern std::string print_array2D(double* arr, uint64_t nq, uint64_t np) {
	std::string result = "";
	result += "################### 2D array bellow ##################";
	for (uint64_t i = 0; i < nq; i++) { //
		for (uint64_t j = 0; j < np; ++j) { 
			result += std::to_string(arr[i * np + j]) + " ";
		}
		result += "\n";
	}
  result += "-------------------------------------\n";
	return result;
}