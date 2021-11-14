/**
 * jacobi-1d-imper.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */

#include "jacobi_1D.hpp"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <omp.h>

 /* Include polybench common header. */
#include <polybench.h>
#include <iostream>

#define LARGE_DATASET

/* Array initialization. */
void init_array(int n,
	DATA_TYPE POLYBENCH_1D(A, N, n),
	DATA_TYPE POLYBENCH_1D(B, N, n))
{
	int i;

	for (i = 0; i < n; i++)
	{
		A[i] = ((DATA_TYPE)i + 2) / n;
		B[i] = ((DATA_TYPE)i + 3) / n;
	}
}

/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static void print_array(int n,
	DATA_TYPE POLYBENCH_1D(A, N, n))

{
	int i;

	for (i = 0; i < n; i++)
	{
		fprintf(stderr, DATA_PRINTF_MODIFIER, A[i]);
		if (i % 20 == 0)
			fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
void kernel_jacobi_1d_imper(int tsteps,
	int n,
	DATA_TYPE POLYBENCH_1D(A, N, n),
	DATA_TYPE POLYBENCH_1D(B, N, n))
{
	int t, i, j;

#pragma scop
	for (t = 0; t < _PB_TSTEPS; t++)
	{
		for (i = 1; i < _PB_N - 1; i++)
			B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);
		for (j = 1; j < _PB_N - 1; j++)
			A[j] = B[j];
	}
#pragma endscop
}

void parallel_jacobi_1d_imper(int tsteps,
        int n,
        DATA_TYPE POLYBENCH_1D(A, N, n),
        DATA_TYPE POLYBENCH_1D(B, N, n))
{

#pragma scop
    for (int t = 0; t < _PB_TSTEPS; t++)
    {
        #pragma omp parallel for
        for (int i = 1; i < _PB_N - 1; i++)
            B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);

        #pragma omp parallel for
        for (int j = 1; j < _PB_N - 1; j++)
            A[j] = B[j];

    }

//    for (int t = 0; t < _PB_TSTEPS; t++)
//    {
//        #pragma omp parallel
//        {
//            int nthreads = omp_get_num_threads();
//            int tid = omp_get_thread_num();
//            int low = (_PB_N - 1) * tid / nthreads;
//            int high = (_PB_N - 1) * (tid + 1) / nthreads;
//            // Make sure we start at 1 and not 0
//            if(low == 0) {
//                low = 1;
//            }
//            for (int i = low; i < high; i++)
//                B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);
//
//            #pragma omp barrier
//            for (int j = 1; j < _PB_N - 1; j++)
//                A[j] = B[j];
//        }
//    }
#pragma endscop
}


