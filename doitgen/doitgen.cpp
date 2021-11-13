#include "doitgen.hpp"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <iostream>

/*
* For every pair of index r, q we access  3 * 256 doubles on the large data set.
* This sums up to 6144 bytes per pair rq of indices. My level 1 cache is
* 640 kB per processor I think. Therefore, 320/3 of such accesses can fit in
* the L1 cache (640kB/6144). We have that sqrt(320/3) is approcimately 10.32. To be 
* safe we can try a blocking window of size 8.
*/
//For now, this doesn't seem to have an effect. This probably because of the coherence traffic.
#define BLOCKING_WINDOW 1

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

	/*#pragma omp parallel for collapse(2)
	for (uint64_t r = 0; r < _PB_NR; r += BLOCKING_WINDOW) {
		for (uint64_t q = 0; q < _PB_NQ; q += BLOCKING_WINDOW) {

			for (uint64_t rr = r; rr < std::min(r + BLOCKING_WINDOW, _PB_NR); ++rr) {
				for (uint64_t qq = q; qq < std::min(q + BLOCKING_WINDOW, _PB_NQ); ++qq) {


					for (uint64_t p = 0; p < _PB_NP; p++) {
						double cur_sum = 0.0;
						#pragma omp parallel for reduction(+:cur_sum)
						for (uint64_t s = 0; s < _PB_NP; s++) {
							cur_sum = cur_sum + A[rr][qq][s] * C4[s][p];
						}
						sum[rr][qq][p] = cur_sum;
					}


					//#pragma omp parallel for
					for (uint64_t p = 0; p < _PB_NR; p++) {
						A[rr][qq][p] = sum[rr][qq][p];
					}
				}
			}
		}
	}*/
	//We reduce false sharing by having a sum array for each thread.
	double* new_sum = new double[omp_get_max_threads() * _PB_NP];

	#pragma omp parallel for collapse(2) shared(new_sum)
	for (uint64_t r = 0; r < _PB_NR; r += BLOCKING_WINDOW) {
		for (uint64_t q = 0; q < _PB_NQ; q += BLOCKING_WINDOW) {
			//for (uint64_t rr = r; rr < std::min(r + BLOCKING_WINDOW, _PB_NR); ++rr) {
				//for (uint64_t qq = q; qq < std::min(q + BLOCKING_WINDOW, _PB_NQ); ++qq) {
					for (uint64_t p = 0; p < _PB_NP; p++) {
						double cur_sum = 0.0;
						for (uint64_t s = 0; s < _PB_NP; s++) {
							cur_sum = cur_sum + A[r][q][s] * C4[s][p];
						}
						new_sum[omp_get_thread_num() * _PB_NP + p] = cur_sum;
						//sum[r][q][p] = cur_sum;
					}

					for (uint64_t p = 0; p < _PB_NR; p++) {
						A[r][q][p] = new_sum[omp_get_thread_num() * _PB_NP + p];
						//A[r][q][p] = sum[r][q][p];
					}
				//}
			//}
			
		}
	}


	delete[]new_sum;
}