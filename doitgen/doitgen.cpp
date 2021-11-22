#include "doitgen.hpp"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <iostream>
#include <cassert>
#define _OPENMP
#include <liblsb.h>

/*
* For every pair of index r, q we access  3 * 256 doubles on the large data set.
* This sums up to 6144 bytes per pair rq of indices. My level 1 cache is
* 640 kB per processor I think. Therefore, 320/3 of such accesses can fit in
* the L1 cache (640kB/6144). We have that sqrt(320/3) is approcimately 10.32. To be
* safe we can try a blocking window of size 8.
*/
//For now, this doesn't seem to have an effect. This probably because of the coherence traffic.
#define BLOCKING_WINDOW 32

/* Include benchmark-specific header. */
/* Default data type is double, default size is 4000. */

/* Array initialization. */
void init_array(uint64_t nr, uint64_t nq, uint64_t np, double* a, double* c4)
{
	uint64_t i, j, k;

	for (i = 0; i < nr; i++) {
		for (j = 0; j < nq; j++) {
			for (k = 0; k < np; k++) {
				A(i, j, k) = ((double)i * j + k) / np;
			}
		}
	}

	for (i = 0; i < np; i++) {
		for (j = 0; j < np; j++) {
			C4(i, j) = ((double)i * j) / np;
		}
	}
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
void kernel_doitgen_seq(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
) {
	uint64_t r, q, p, s;

#pragma scop
	for (r = 0; r < nr; r++)
		for (q = 0; q < nq; q++)
		{
			for (p = 0; p < np; p++)
			{
				//sum[r][q][p] = 0;
				SUM(r, q, p) = 0;
				for (s = 0; s < np; s++) {
					//sum[r][q][p] = sum[r][q][p] + A[r][q][s] * C4[s][p];
					SUM(r, q, p) = SUM(r, q, p) + A(r, q, s) * C4(s, p);
				}
			}
			/*
			* Are we sure this is correct ? I think the boundary condition for the
			* loop variable p should be _PB_NP since the last dimension of sum and
			* A has size np. Maybe this is just how the program should work.
			*/
			for (p = 0; p < nr; p++) {
				A(r, q, p) = SUM(r, q, p);
				//A[r][q][p] = sum[r][q][p];
			}
		}
#pragma endscop
}

void kernel_doitgen_openmp(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
) {
	//We reduce false sharing by having a sum array for each thread.
	//double* new_sum = new double[omp_get_max_threads() * _PB_NP];

	/*
	* Maybe we should do blocking on C4 because we access a lot of those values.
	* I will try this.
	* Blocking on the 114-121 lines of code.
	*/
	double* new_sum = new double[np * omp_get_max_threads()];

#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {
		for (uint64_t q = 0; q < nq; q++) {



			/*
			* This is the dot product bewtween the row A[r][q] and the column C4[p]
			*/
			for (uint64_t p = 0; p < np; p++) {
				double cur_sum = 0.0;
				for (uint64_t s = 0; s < np; s++) {
					//This needs the old values of A
					//cur_sum = cur_sum + A[r][q][s] * C4[s][p];
					cur_sum = cur_sum + A(r, q, s) * C4(s, p);
				}
				new_sum[omp_get_thread_num() * np + p] = cur_sum;
			}

			for (uint64_t p = 0; p < nr; p++) {
				//A[r][q][p] = new_sum[omp_get_thread_num() * np + p];
				A(r, q, p) = new_sum[omp_get_thread_num() * np + p];
			}

		}
	}
	delete[]new_sum;
}

void kernel_doitgen_experimental(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
) {
	//We reduce false sharing by having a sum array for each thread.
	//double* new_sum = new double[omp_get_max_threads() * _PB_NP];

	/*
	* Maybe we should do blocking on C4 because we access a lot of those values.
	* I will try this.
	* Blocking on the 114-121 lines of code.
	*/
	//double* new_sum = new double[np * omp_get_max_threads()];

#pragma omp parallel for num_threads(16)
	for (uint64_t r = 0; r < nr; r++) {
		for (uint64_t q = 0; q < nq; q += BLOCKING_WINDOW) {
			for (uint64_t p = 0; p < np; p += BLOCKING_WINDOW) {

				for (uint64_t qq = q; qq < q + BLOCKING_WINDOW; ++qq) {
					for (uint64_t pp = p; pp < p + BLOCKING_WINDOW; ++pp) {
						double cur_sum = 0.0;
						for (uint64_t s = 0; s < np; s++) {
							cur_sum = cur_sum + A_IN(r, qq, s) * C4(s, pp);
						}
						if (p < nr) {
							A_OUT(r, qq, pp) = cur_sum;
						}
					}
				}
			}
		}
	}
}

void kernel_doitgen_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
) {

#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {
		/*for (uint64_t q = 0; q < nq; q += BLOCKING_WINDOW) {
			for (uint64_t p = 0; p < np; p += BLOCKING_WINDOW) {

				for (uint64_t s = 0; s < np; s += BLOCKING_WINDOW) {
					for (uint64_t qq = q; qq < q + BLOCKING_WINDOW; ++qq) {
						for (uint64_t pp = p; pp < p + BLOCKING_WINDOW; ++pp) {
							double cur_sum = SUM(r, qq, pp);
							for (uint64_t ss = s; ss < s + BLOCKING_WINDOW; ++ss) {
								cur_sum += A_IN(r, qq, ss) * C4(ss, pp);
							}
							SUM(r, qq, pp) = cur_sum;
							if (pp < nr) {
								A_OUT(r, q, pp) = SUM(r, qq, pp);

							}

						}
					}

				}



			}
		}*/
		/*
		* Only works for now because the matrix size is a facor of the BLOCKING_WINDOW
		*/
		for (uint64_t p = 0; p < np; p += BLOCKING_WINDOW) {
			for (uint64_t s = 0; s < np; s += BLOCKING_WINDOW) {
				for (uint64_t q = 0; q < nq; q++) {
					for (uint64_t pp = p; pp < p + BLOCKING_WINDOW; pp++) {
						//double cur_sum = SUM(r, q, pp);
						for (uint64_t ss = s; ss < s + BLOCKING_WINDOW; ss++) {
							//cur_sum += A_IN(r, q, ss) * C4(ss, pp);
							A_OUT(r, q, pp) += A_IN(r, q, ss) * C4(ss, pp);
						}
						//SUM(r, q, pp) = cur_sum;
						//A_OUT(r, q, pp) = SUM(r, q, pp) * (pp < nr) + A_IN(r, q, pp) * (pp >= nr);
					}
				}
			}
		}

	}
}

void kernel_doitgen_no_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
) {

	#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {
		for (uint64_t q = 0; q < nq; q++) {
			for (uint64_t s = 0; s < np; s++) {
				for (uint64_t p = 0; p < np; p++) {
					A_OUT(r, q, p) += A_IN(r, q, s) * C4(s, p);
				}
			}
		}
	}

}

void kernel_doitgen_transpose(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
) {
#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {
		for (uint64_t q = 0; q < nq; q ++) {
			for (uint64_t p = 0; p < np; p ++) {
				//double cur_sum = 0.0;
				for (uint64_t s = 0; s < np; s++) {
					A_OUT(r, q, p) += A_IN(r, q, s) * C4(p, s);
				}
				/*if (p < nr) {
					A_OUT(r, q, p) = cur_sum;
				}*/
			}
		}
	}
}

void kernel_doitgen_transpose_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
) {
	#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {
		for (uint64_t p = 0; p < np; p += BLOCKING_WINDOW) {
			for (uint64_t s = 0; s < np; s += BLOCKING_WINDOW) {
				for (uint64_t q = 0; q < nq; q++) {
					for (uint64_t pp = p; pp < p + BLOCKING_WINDOW; pp++) {
						//double cur_sum = SUM(r, q, pp);
						for (uint64_t ss = s; ss < s + BLOCKING_WINDOW; ss++) {
							A_OUT(r, q, pp) += A_IN(r, q, ss) * C4(pp, ss);
						}
						/*SUM(r, q, pp) = cur_sum;
						if (pp < nr) {
							A_OUT(r, q, pp) = SUM(r, q, pp);
						}*/
						//A_OUT(r, q, pp) = SUM(r, q, pp) * (pp < nr) + A_IN(r, q, pp) * (pp >= nr);
					}
				}
			}
		}
	}
}

/**
 * @brief 
 * 
 * @param nr 
 * @param nq 
 * @param np 
 * @param a_in 
 * @param a_out 
 * @param c4 
 * @param sum 
 * 
 * Processes are instances of the program that openMpi runs (communicator_size).
 * Processes can be organized into logical groups. The communicators
 * are objects that handle communications between processes. There also exists
 * intra-communicator for communication inside a group of process and inter-communicator
 * for communication of processes in two distinct group. We have a single group with the
 * communicator MPI_COMM_WORLD. The rank of a process is relative to the group. If we split
 * our processes in two groups, they will have a rank in the original MPI_COMM_WORLD and
 * in the new one.
 */
void kernel_doitgen_mpi(MPI_Comm bench_comm, uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
) {

	int num_proc = 0;
	int rank = 0;

	//Get the total number of processes available for the work
	MPI_Comm_size(bench_comm, &num_proc);
	MPI_Comm_rank(bench_comm, &rank);
	
	uint64_t chunk_size = nr / num_proc;
	uint64_t leftover = nr % num_proc; // we compute the imbalance in jobs
	uint64_t normal = num_proc - leftover; // the amount of processes that will not have an additional job
	uint64_t imbalanced_start = normal * chunk_size; //start index of the increased jobs

	uint64_t l = 0;
	uint64_t u = 0;

	// TODO check 
	if ((uint64_t) rank < normal) {
		l = rank * chunk_size;
		u = (rank + 1) * chunk_size;
	} else { // imbalanced workload process
		l = (rank - normal) * (chunk_size + 1) + imbalanced_start;
		u = (rank - normal + 1) * (chunk_size + 1) + imbalanced_start;
	}
  	
	uint64_t r = 0, q = 0, p = 0, s = 0;
	
	// instead of r in [0, nr], each process do its part of the job
	for (r = l; r < u; r++) {

		for (q = 0; q < nq; q++)  {
			for (p = 0; p < np; p++)  {
				sum[p] = 0;
				for (s = 0; s < np; s++) {
					sum[p] += A(r, q, s) * C4(s, p);
				}
			}

			for (p = 0; p < np; p++) {
				A(r, q, p) = sum[p];
			}
		}

	}



}

void kernel_doitgen_mpi(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
) {
	kernel_doitgen_mpi(MPI_COMM_WORLD, nr, nq, np, a, c4, sum);
}


void kernel_doitgen_mpi_init(MPI_Win* shared_window, uint64_t nr, uint64_t nq, uint64_t np, double** a, double** c4, double** sum) {
    
	int num_processors = 0;
	int rank = 0;

    //Get the total number of processes available for the work (in the world)
	MPI_Comm_size(MPI_COMM_WORLD, &num_processors);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//PROCESS_MESSAGE(rank, "starting :)");

	MPI_Aint a_size = nr * nq * np * sizeof(double);
	MPI_Aint c4_size = np * np * sizeof(double);
	int displacement_unit = sizeof(double); //placeholder

	if (rank == 0) {
      
		MPI_Win_allocate_shared(
			a_size, 
			sizeof(double),
			MPI_INFO_NULL,
			MPI_COMM_WORLD, 
			a, 
			shared_window
		);

		MPI_Win_allocate_shared(
			c4_size, 
			sizeof(double),
			MPI_INFO_NULL,
			MPI_COMM_WORLD, 
			c4, 
			shared_window
		);


	} else {

		MPI_Win_allocate_shared(
			0, 
		   	sizeof(double), 
		  	MPI_INFO_NULL,
        	MPI_COMM_WORLD,
			a,
			shared_window
		);

		MPI_Win_shared_query(*shared_window, 0, &a_size, &displacement_unit, a);

		displacement_unit = 0;
		MPI_Win_allocate_shared(
			0, 
		   	sizeof(double), 
		  	MPI_INFO_NULL,
        	MPI_COMM_WORLD,
			c4,
			shared_window
		);

		MPI_Win_shared_query(*shared_window, 0, &c4_size, &displacement_unit, c4);
	
	}

	MPI_Barrier(MPI_COMM_WORLD); // wait that all processes have requested thier memory

	// sum is private too (everyone allocates its own)
	*sum = (double*) MPI::Alloc_mem(np * sizeof(double), MPI_INFO_NULL);
	memset(*sum, 0.0, np);

	if (rank == 0) {
		init_array(nr, nq, np, *a, *c4);
	}

	MPI_Barrier(MPI_COMM_WORLD); // wait that all threads have seen initialization of a and C4
}

void kernel_doitgen_mpi_clean(MPI_Win* shared_window, double** sum) {
	MPI_Free_mem(*sum);
	*sum = 0;
	MPI_Win_free(shared_window);
	*shared_window = 0;
}