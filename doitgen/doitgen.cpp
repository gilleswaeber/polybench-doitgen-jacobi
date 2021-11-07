
#define LARGE_DATASET
#include "doitgen.hpp"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <iostream>

/* Include benchmark-specific header. */
/* Default data type is double, default size is 4000. */

/* Array initialization. */
void init_array(uint64_t nr, uint64_t nq, uint64_t np,
	DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np),
	DATA_TYPE POLYBENCH_2D(C4, NP, NP, np, np))
{
	uint64_t i, j, k;

	for (i = 0; i < nr; i++)
		for (j = 0; j < nq; j++)
			for (k = 0; k < np; k++)
				A[i][j][k] = ((DATA_TYPE)i * j + k) / np;
	for (i = 0; i < np; i++)
		for (j = 0; j < np; j++)
			C4[i][j] = ((DATA_TYPE)i * j) / np;
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
void kernel_doitgen(uint64_t nr, uint64_t nq, uint64_t np,
	DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np),
	DATA_TYPE POLYBENCH_2D(C4, NP, NP, np, np),
	DATA_TYPE POLYBENCH_3D(sum, NR, NQ, NP, nr, nq, np))
{
	uint64_t r, q, p, s;

#pragma scop
	for (r = 0; r < _PB_NR; r++)
		for (q = 0; q < _PB_NQ; q++)
		{
			for (p = 0; p < _PB_NP; p++)
			{
				sum[r][q][p] = 0;
				for (s = 0; s < _PB_NP; s++)
					sum[r][q][p] = sum[r][q][p] + A[r][q][s] * C4[s][p];
			}
			/*
			* Are we sure this is correct ? I think the boundary condition for the
			* loop variable p should be _PB_NP since the last dimension of sum and
			* A has size np. Maybe this is just how the program should work.
			*/
			for (p = 0; p < _PB_NR; p++)
				A[r][q][p] = sum[r][q][p];
		}
#pragma endscop
}

void parallel_doitgen(uint64_t nr, uint64_t nq, uint64_t np,
	DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np),
	DATA_TYPE POLYBENCH_2D(C4, NP, NP, np, np),
	DATA_TYPE POLYBENCH_3D(sum, NR, NQ, NP, nr, nq, np)) {

	#pragma omp parallel for collapse(2)
	for (uint64_t r = 0; r < _PB_NR; r++) {
		for (uint64_t q = 0; q < _PB_NQ; q++) {
			for (uint64_t p = 0; p < _PB_NP; p++) {
				double cur_sum = 0.0;
				for (uint64_t s = 0; s < _PB_NP; s++) {
					cur_sum = cur_sum + A[r][q][s] * C4[s][p];
				}
				sum[r][q][p] = cur_sum;
			}


			//#pragma omp parallel for
			for (uint64_t p = 0; p < _PB_NR; p++) {
				A[r][q][p] = sum[r][q][p];
			}


		}
	}
}