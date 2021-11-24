/**
 * Taken from polybench by Louis
 * 
 */

#include "utils.hpp"

#include <string.h>
#include <iostream>
#include <assert.h>

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