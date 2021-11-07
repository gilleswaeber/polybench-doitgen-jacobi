
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
void init_array(int nr, int nq, int np,
	DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np),
	DATA_TYPE POLYBENCH_2D(C4, NP, NP, np, np))
{
	int i, j, k;

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
void kernel_doitgen(int nr, int nq, int np,
	DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np),
	DATA_TYPE POLYBENCH_2D(C4, NP, NP, np, np),
	DATA_TYPE POLYBENCH_3D(sum, NR, NQ, NP, nr, nq, np))
{
	int r, q, p, s;

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
			for (p = 0; p < _PB_NR; p++)
				A[r][q][p] = sum[r][q][p];
		}
#pragma endscop
}

void parallel_doitgen(int nr, int nq, int np,
	DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np),
	DATA_TYPE POLYBENCH_2D(C4, NP, NP, np, np),
	DATA_TYPE POLYBENCH_3D(sum, NR, NQ, NP, nr, nq, np)) {
	int r, q, p, s;

	for (r = 0; r < _PB_NR; r++) {
		for (q = 0; q < _PB_NQ; q++) {

			
			#pragma omp parallel for
			for (p = 0; p < _PB_NP; p++) {
				double cur_sum = 0.0;
				for (s = 0; s < _PB_NP; s++) {
					cur_sum = cur_sum + A[r][q][s] * C4[s][p];
				}
				sum[r][q][p] = cur_sum;
			}

			for (p = 0; p < _PB_NR; p++) {
				A[r][q][p] = sum[r][q][p];
			}
		}
	}
}