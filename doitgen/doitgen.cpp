#include "doitgen.hpp"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <iostream>
#include <cassert>
#include <liblsb.h>
#include <string>
#include <immintrin.h>
#include <liblsb.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <errno.h>

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

void init_C4(uint64_t np, double* c4) {
	uint64_t i, j;
	for (i = 0; i < np; i++) {
		for (j = 0; j < np; j++) {
			C4(i, j) = ((double)i * j) / np;
		}
	}
}

std::string print_array2D_test(double* arr, uint64_t nq, uint64_t np) {
	std::string result = "";
	result += "################### 2D array bellow ######################\n";
	for (uint64_t i = 0; i < nq; i++) {
		for (uint64_t j = 0; j < np; j++) {
			result += std::to_string(ARR_2D(arr, nq, i, j)) + " ";
		}
		result += "\n";
	}
	result += "-------------------------------------\n";
	return result;
}

void init_A_slice(uint64_t nq, uint64_t np, double* a, uint64_t i) {

	uint64_t j, k;

	for (j = 0; j < nq; j++) {
		for (k = 0; k < np; k++) {
			double result = ((double)i * j + k) / np;
			A_SLICE(j, k) = result;
		}
	}

}

/**
 * @brief
 *
 *
 * @param nq
 * @param np
 * @param a
 * @param i_lower inclusive
 * @param i_upper exclusive
 */
void init_A_slice_batch(uint64_t nq, uint64_t np, double* a, uint64_t i_lower, uint64_t i_upper) {

	uint64_t j, k;

	for (uint64_t i = i_lower; i < i_upper; i++) {
		for (j = 0; j < nq; j++) {
			for (k = 0; k < np; k++) {
				double result = ((double)i * j + k) / np;
				//A_SLICE(j, k) = result;
				ARR_3D(a, i_upper - i_lower, nq, np, i - i_lower, j, k) = result;
			}
		}
	}

}

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

	for (r = 0; r < nr; r++) {

		for (q = 0; q < nq; q++) {

			for (p = 0; p < np; p++) {
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
#pragma endscop
}

void kernel_doitgen_polybench_parallel(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
) {

#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t q = 0; q < nq; q++) {

			for (uint64_t p = 0; p < np; p++) {
				sum[omp_get_thread_num() * np + p] = 0;
				for (uint64_t s = 0; s < np; s++) {
					sum[omp_get_thread_num() * np + p] += A(r, q, s) * C4(s, p);
				}
			}

			for (uint64_t p = 0; p < np; p++) {
				A(r, q, p) = sum[omp_get_thread_num() * np + p];
			}
		}

	}
}

void kernel_doitgen_polybench_parallel_local_sum(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
) {
#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {
		for (uint64_t q = 0; q < nq; q++) {



			/*
			* This is the dot product bewtween the row A[r][q] and the column C4[p]
			*/
			for (uint64_t p = 0; p < np; p++) {
				for (uint64_t s = 0; s < np; s++) {
					//This needs the old values of A
					//cur_sum = cur_sum + A[r][q][s] * C4[s][p];
					sum[omp_get_thread_num() * np + p] += A(r, q, s) * C4(s, p);
				}
			}
			memcpy(&(A(r, q, 0)), &(sum[omp_get_thread_num() * np]), np * sizeof(double));
			memset(&(sum[omp_get_thread_num() * np]), 0, np * sizeof(double));
		}
	}
}

void kernel_doitgen_transpose(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4
) {
#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {
		for (uint64_t q = 0; q < nq; q++) {
			for (uint64_t p = 0; p < np; p++) {
				for (uint64_t s = 0; s < np; s++) {
					A_OUT(r, q, p) += A_IN(r, q, s) * C4(p, s);
				}
			}
		}
	}
}

void kernel_doitgen_transpose_local_sum(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* sum,
	double* c4
) {
#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {
		for (uint64_t q = 0; q < nq; q++) {
			for (uint64_t p = 0; p < np; p++) {
				for (uint64_t s = 0; s < np; s++) {
					sum[omp_get_thread_num() * np + p] += A_IN(r, q, s) * C4(p, s);
				}
			}
			memcpy(&(A_IN(r, q, 0)), &(sum[omp_get_thread_num() * np]), np * sizeof(double));
			memset(&(sum[omp_get_thread_num() * np]), 0, np * sizeof(double));
		}
	}
}

void kernel_doitgen_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	uint64_t blocking_window
) {

#pragma omp parallel for
	for (uint64_t r = 0; r < nr; ++r) {
		for (uint64_t q = 0; q < nq; q += blocking_window) {
			for (uint64_t p = 0; p < np; p += blocking_window) {
				for (uint64_t k = 0; k < np; k += blocking_window) {
					for (uint64_t qq = q; qq < q + blocking_window; ++qq) {
						for (uint64_t pp = p; pp < p + blocking_window; ++pp) {
							for (uint64_t kk = k; kk < k + blocking_window; ++kk) {
								A_OUT(r, qq, pp) += A_IN(r, qq, kk) * C4(kk, pp);
							}
						}
					}
				}
			}
		}
	}
}

void kernel_doitgen_inverted_loop(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4
) {

#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t q = 0; q < nq; q++) {
			for (uint64_t k = 0; k < np; k++) {
				for (uint64_t p = 0; p < np; p++) {
					A_OUT(r, q, p) += A_IN(r, q, k) * C4(k, p);
				}
			}
		}
	}
}

void kernel_doitgen_inverted_loop_local_sum(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* sum,
	double* c4
) {

#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t q = 0; q < nq; q++) {
			for (uint64_t k = 0; k < np; k++) {
				for (uint64_t p = 0; p < np; p++) {
					SUM(omp_get_thread_num(), q, p) += A(r, q, k) * C4(k, p);
				}
			}
		}

		memcpy(&A(r, 0, 0), &SUM(omp_get_thread_num(), 0, 0), nq * np * sizeof(double));
		memset(&SUM(omp_get_thread_num(), 0, 0), 0, nq * np * sizeof(double));
	}
}

void kernel_doitgen_inverted_loop_local_sum_1D(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* sum,
	double* c4
) {

#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t q = 0; q < nq; q++) {
			for (uint64_t k = 0; k < np; k++) {
				for (uint64_t p = 0; p < np; p++) {
					sum[omp_get_thread_num() * np + p] += A(r, q, k) * C4(k, p);
				}
			}
			memcpy(&(A(r, q, 0)), &(sum[omp_get_thread_num() * np]), np * sizeof(double));
			memset(&(sum[omp_get_thread_num() * np]), 0, np * sizeof(double));
		}
	}
}

void kernel_doitgen_inverted_loop_blocking(uint64_t nr, uint64_t nq, uint64_t np, double* a_in,
	double* a_out, double* c4, uint64_t blocking_size) {

#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t i = 0; i < nq; i += blocking_size) {
			for (uint64_t k = 0; k < np; k += blocking_size) {
				for (uint64_t j = 0; j < np; j += blocking_size) {
					for (uint64_t ii = i; ii < i + blocking_size; ii++) {
						for (uint64_t kk = k; kk < k + blocking_size; kk++) {
							for (uint64_t jj = j; jj < j + blocking_size; jj++) {
								A_OUT(r, ii, jj) += A_IN(r, ii, kk) * C4(kk, jj);
							}
						}
					}

				}
			}
		}

	}
}

void kernel_doitgen_inverted_loop_avx2(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4
) {
#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t q = 0; q < nq; q++) {
			for (uint64_t k = 0; k < np; k++) {
				__m256d a_in_val = _mm256_set1_pd(A_IN(r, q, k));
				for (uint64_t p = 0; p < np; p += 4) {
					__m256d a_out_val = _mm256_load_pd(&(A_OUT(r, q, p)));
					__m256d c4_val = _mm256_load_pd(&(C4(k, p)));

					__m256d res = _mm256_fmadd_pd(a_in_val, c4_val, a_out_val);

					_mm256_store_pd(&(A_OUT(r, q, p)), res);
				}
			}
		}
	}
}

void kernel_doitgen_inverted_loop_avx2_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	uint64_t blocking_size
) {
#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t q = 0; q < nq; q += blocking_size) {
			for (uint64_t k = 0; k < np; k += blocking_size) {
				for (uint64_t p = 0; p < np; p += blocking_size) {
					for (uint64_t qq = q; qq < q + blocking_size; qq++) {
						for (uint64_t kk = k; kk < k + blocking_size; kk++) {
							__m256d a_in_val = _mm256_set1_pd(A_IN(r, qq, kk));
							for (uint64_t pp = p; pp < p + blocking_size; pp += 4) {
								__m256d a_out_val = _mm256_load_pd(&(A_OUT(r, qq, pp)));
								__m256d c4_val = _mm256_load_pd(&(C4(kk, pp)));

								__m256d res = _mm256_fmadd_pd(a_in_val, c4_val, a_out_val);

								_mm256_store_pd(&(A_OUT(r, qq, pp)), res);
							}
						}
					}
				}
			}
		}
	}
}

void kernel_doitgen_inverted_loop_avx2_blocking_local_sum_2D(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* sum,
	double* c4,
	uint64_t blocking_size
) {
#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t q = 0; q < nq; q += blocking_size) {
			for (uint64_t k = 0; k < np; k += blocking_size) {
				for (uint64_t p = 0; p < np; p += blocking_size) {
					for (uint64_t qq = q; qq < q + blocking_size; qq++) {
						for (uint64_t kk = k; kk < k + blocking_size; kk++) {
							__m256d a_in_val = _mm256_set1_pd(A_IN(r, qq, kk));
							for (uint64_t pp = p; pp < p + blocking_size; pp += 4) {
								__m256d a_out_val = _mm256_load_pd(&(SUM(omp_get_thread_num(), qq, pp)));
								__m256d c4_val = _mm256_load_pd(&(C4(kk, pp)));

								__m256d res = _mm256_fmadd_pd(a_in_val, c4_val, a_out_val);

								_mm256_store_pd(&(SUM(omp_get_thread_num(), qq, pp)), res);
							}
						}
					}
				}
			}
		}
		memcpy(&A_IN(r, 0, 0), &SUM(omp_get_thread_num(), 0, 0), nq * np * sizeof(double));
		memset(&SUM(omp_get_thread_num(), 0, 0), 0, nq * np * sizeof(double));
	}
}

void kernel_doitgen_inverted_loop_avx2_local_sum(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* sum,
	double* c4
) {
#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t q = 0; q < nq; q++) {
			for (uint64_t k = 0; k < np; k++) {
				__m256d a_in_val = _mm256_set1_pd(A_IN(r, q, k));
				for (uint64_t p = 0; p < np; p += 4) {
					__m256d a_out_val = _mm256_load_pd(&(SUM(omp_get_thread_num(), q, p)));
					__m256d c4_val = _mm256_load_pd(&(C4(k, p)));

					__m256d res = _mm256_fmadd_pd(a_in_val, c4_val, a_out_val);

					_mm256_store_pd(&(SUM(omp_get_thread_num(), q, p)), res);
				}
			}
		}
		memcpy(&A_IN(r, 0, 0), &SUM(omp_get_thread_num(), 0, 0), nq * np * sizeof(double));
		memset(&SUM(omp_get_thread_num(), 0, 0), 0, nq * np * sizeof(double));
	}
}

void kernel_doitgen_inverted_loop_avx2_local_sum_1D(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* sum,
	double* c4
) {
#pragma omp parallel for
	for (uint64_t r = 0; r < nr; r++) {

		for (uint64_t q = 0; q < nq; q++) {
			for (uint64_t k = 0; k < np; k++) {
				__m256d a_in_val = _mm256_set1_pd(A_IN(r, q, k));
				for (uint64_t p = 0; p < np; p += 4) {
					__m256d a_out_val = _mm256_load_pd(&(sum[omp_get_thread_num() * np + p]));
					__m256d c4_val = _mm256_load_pd(&(C4(k, p)));

					__m256d res = _mm256_fmadd_pd(a_in_val, c4_val, a_out_val);

					_mm256_store_pd(&(sum[omp_get_thread_num() * np + p]), res);
				}
			}
			memcpy(&(A_IN(r, q, 0)), &(sum[omp_get_thread_num() * np]), np * sizeof(double));
			memset(&(sum[omp_get_thread_num() * np]), 0, np * sizeof(double));
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



///////////////////////////////////////////////// UTILS MPI ///////////////////////////////////

std::string get_overall_file_name(char** argv, uint64_t num_processor) {
	std::string benchmark_name = argv[2];
	std::string processor_model = argv[3];
	uint64_t run_index = strtoull(argv[4], nullptr, 10);

	std::string result = std::string("lsb.") + get_benchmark_lsb_name(benchmark_name, processor_model, num_processor) + std::string("_") + std::to_string(run_index) + std::string("_overall.r0");
	return result;
}

void mpi_lsb_benchmark_startup(char** argv, int argc, uint64_t* nr, uint64_t* nq, uint64_t* np, char** output_path, mpi_kernel_func* selected_kernel) {

	MPI_Init(nullptr, nullptr);

	assert(argc == 8);

	*output_path = argv[1];
	std::string benchmark_name = argv[2];
	std::string processor_model = argv[3];

	//mpi_kernel_func selected_kernel;
	bool found_kernel = find_benchmark_kernel_by_name(benchmark_name, selected_kernel);
	assert(found_kernel);

	std::cout << "launching benchmark for " << benchmark_name << std::endl;

	remove(*output_path);

	uint64_t run_index = strtoull(argv[4], nullptr, 10);

	*nr = strtoull(argv[5], nullptr, 10);
	*nq = strtoull(argv[6], nullptr, 10);
	*np = strtoull(argv[7], nullptr, 10);

	int num_proc, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	LSB_Init(get_benchmark_lsb_name(benchmark_name, processor_model, num_proc).c_str(), 0);

	LSB_Set_Rparam_long("NR", *nr);
	LSB_Set_Rparam_long("NQ", *nq);
	LSB_Set_Rparam_long("NP", *np);
	LSB_Set_Rparam_long("num_processes", num_proc);
	LSB_Set_Rparam_long("run_index", run_index);

	LSB_Set_Rparam_string("benchmark_type", argv[2]);
	LSB_Set_Rparam_string("processor_model", argv[3]);

	if (rank == 0) {
		std::cout << "starting benchmark without file" << nr << "x" << nq << "x" << np << std::endl;
		std::cout << "num process = " << num_proc << std::endl;
	}

}

void mpi_lsb_benchmark_finalize() {
	LSB_Finalize();
	MPI_Finalize();
}

/*this function takes C4 and transposes it inplace */
void tranpose_C4(uint64_t np, double* c4) {

	assert(c4 != nullptr);

	for (uint64_t i = 0; i < np; i++) {
		for (uint16_t j = i + 1; j < np; j++) {
			double tmp = c4[i * np + j];
			c4[i * np + j] = c4[j * np + i];
			c4[j * np + i] = tmp;
		}
	}

}

void doitgen_kernel_mpi_init(uint64_t nr, uint64_t nq, uint64_t np, int* num_proc, int* rank, double** a, double** sum, double** c4, uint64_t* l, uint64_t* u) {

	MPI_Comm_size(MPI_COMM_WORLD, num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, rank);

	if (*rank == 0) {
		std::cout << "num proc = " << *num_proc << " for size " << nr << "x" << nq << "x" << np << std::endl;
	}

	*a = (double*)calloc(nq * np, sizeof(double));
	*sum = (double*)calloc(np, sizeof(double));
	*c4 = (double*)calloc(np * np, sizeof(double));

	assert(*a != nullptr);
	assert(*sum != nullptr);
	assert(*c4 != nullptr);

	init_C4(np, *c4);
	memset(*sum, 0, np);

	uint64_t chunk_size = nr / *num_proc;
	uint64_t leftover = nr % *num_proc; // we compute the imbalance in jobs
	uint64_t normal = *num_proc - leftover; // the amount of processes that will not have an additional job
	uint64_t imbalanced_start = normal * chunk_size; //start index of the increased jobs

	if ((uint64_t)*rank < normal) {
		*l = *rank * chunk_size;
		*u = (*rank + 1) * chunk_size;
	}
	else { // imbalanced workload process
		*l = (*rank - normal) * (chunk_size + 1) + imbalanced_start;
		*u = (*rank - normal + 1) * (chunk_size + 1) + imbalanced_start;
	}
}

void doitgen_kernel_mpi_init(uint64_t nr, uint64_t nq, uint64_t np, int* num_proc, int* rank, double** a, double** sum, double** c4, uint64_t* l, uint64_t* u, uint64_t a_slices_per_batch) {

	MPI_Comm_size(MPI_COMM_WORLD, num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, rank);

	if (*rank == 0) {
		std::cout << "num proc = " << *num_proc << " for size " << nr << "x" << nq << "x" << np << std::endl;
	}

	*a = (double*)calloc(a_slices_per_batch * nq * np, sizeof(double));
	*sum = (double*)calloc(np, sizeof(double));
	*c4 = (double*)calloc(np * np, sizeof(double));

	assert(*a != nullptr);
	assert(*sum != nullptr);
	assert(*c4 != nullptr);

	init_C4(np, *c4);
	memset(*sum, 0, np);

	uint64_t chunk_size = nr / *num_proc;
	uint64_t leftover = nr % *num_proc; // we compute the imbalance in jobs
	uint64_t normal = *num_proc - leftover; // the amount of processes that will not have an additional job
	uint64_t imbalanced_start = normal * chunk_size; //start index of the increased jobs

	if ((uint64_t)*rank < normal) {
		*l = *rank * chunk_size;
		*u = (*rank + 1) * chunk_size;
	}
	else { // imbalanced workload process
		*l = (*rank - normal) * (chunk_size + 1) + imbalanced_start;
		*u = (*rank - normal + 1) * (chunk_size + 1) + imbalanced_start;
	}
}


void mpi_io_init_file(uint64_t nq, uint64_t np, const char* output_path, MPI_File* file, uint64_t l, uint64_t u) {

	assert(file != nullptr);

	MPI_Datatype arraytype;

	MPI_Offset disp = nq * np * sizeof(double) * l;
	MPI_Offset end = nq * np * sizeof(double) * u;

	MPI_Type_contiguous(end - disp, MPI_DOUBLE, &arraytype);
	MPI_Type_commit(&arraytype);
	MPI_File_open(MPI_COMM_WORLD, output_path, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, file);

	MPI_File_set_view(*file, disp, MPI_DOUBLE, arraytype, "native", MPI_INFO_NULL);

	MPI_Barrier(MPI_COMM_WORLD);

}

void mpi_clean_up(MPI_File* file, double* a, double* sum, double* c4) {

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_File_close(file);
	if (a != nullptr)
		free(a);
	if (sum != nullptr)
		free(sum);
	if (c4 != nullptr)
		free(c4);

	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0) {
		PROCESS_MESSAGE(rank, "exiting!");
	}
}


uint64_t get_elapsed_us(std::chrono::high_resolution_clock::time_point& start, std::chrono::high_resolution_clock::time_point& end) {
	auto start_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(start.time_since_epoch());
	auto end_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end.time_since_epoch());
	return end_microseconds.count() - start_microseconds.count();
}

std::string get_benchmark_lsb_name(const std::string& benchmark_name, const std::string& processor_model, int num_processors) {
	std::string result = "";
	result += std::string("doitgen_") + benchmark_name + std::string("_") + processor_model + std::string("_") + std::to_string(num_processors);
	return result;
}

void flush_cache_epyc()
{
	//taken from polybench
	unsigned long long cs = 256000000 / (unsigned long long)sizeof(double);
	double* flush = (double*)calloc(cs, sizeof(double));
	unsigned long long i;
	double tmp = 0.0;
	for (i = 0; i < cs; i++)
		tmp += flush[i];
	//This is to prevent compiler optimizations
	if (tmp > 10.0) {
		std::cout << "Fail !" << std::endl;
	}
	free((void*)flush);
}

void mpi_write_overall(const std::string& file_name, const std::string& benchmark_name, uint64_t run_index, uint64_t elapsed) {

	int num_proc;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {

		std::vector<uint64_t> overalls(num_proc, 0);
		overalls[rank] = elapsed;

		//get all other threads measurement
		for (int i = 1; i < num_proc; i++) {

			uint64_t other = 0;

			MPI_Recv(
				&other,
				1,
				MPI_UNSIGNED_LONG,
				i,
				0,
				MPI_COMM_WORLD,
				MPI_STATUS_IGNORE
			);

			overalls[i] = other;

		}

		std::sort(overalls.begin(), overalls.end());

		uint64_t min = overalls[0];
		uint64_t max = overalls[num_proc - 1];
		uint64_t median;

		if (num_proc % 2 == 1) {
			median = overalls[num_proc / 2];
		}
		else {
			median = (overalls[(num_proc / 2) - 1] + overalls[num_proc / 2]) / 2;
		}

		std::cout << "min=" << min << " median=" << median << " max=" << max << std::endl;

		std::string line = "";
		line += benchmark_name + "\t" + std::to_string(num_proc) + "\t" + std::to_string(run_index) + "\t" + std::to_string(min) + "\t" + std::to_string(median) + "\t" + std::to_string(max) + "\n";
		std::string header = "benchmark_type\tnum_processes\trun_index\tmin\tmedian\tmax\n";
		FILE* fp = fopen(file_name.c_str(), "w");
		fputs(header.c_str(), fp);
		fputs(line.c_str(), fp);
		fclose(fp);

	}
	else {

		MPI_Send(
			&elapsed,
			1,
			MPI_UNSIGNED_LONG,
			0,
			0,
			MPI_COMM_WORLD
		);

	}

	MPI_Barrier(MPI_COMM_WORLD);

	MASTER_MESSAGE("Written overall!");

}

//////////////////////////////////////////////////// MPI KERNELS ////////////////////////////////////////////


uint64_t kernel_doitgen_mpi_write_1(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path) {

	int num_proc, rank;
	double* a = 0;
	double* sum = 0;
	double* c4 = 0;
	uint64_t l = 0, u = 0;

	doitgen_kernel_mpi_init(nr, nq, np, &num_proc, &rank, &a, &sum, &c4, &l, &u);

	MPI_Offset offset;
	MPI_File file;
	MPI_File_open(MPI_COMM_WORLD, output_path, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);


	flush_cache_epyc();


	MPI_Barrier(MPI_COMM_WORLD);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();


	for (uint64_t i = l; i < u; i++) {
		LSB_Res();
		offset = nq * np * sizeof(double) * i;
		MPI_File_write_at(file, offset, a, nq * np, MPI_DOUBLE, MPI_STATUS_IGNORE);
		LSB_Rec(0);
	}


	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	uint64_t elapsed = get_elapsed_us(start, end);

	mpi_clean_up(&file, a, sum, c4);
	return elapsed;
}

uint64_t kernel_doitgen_mpi_write_2(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path) {

	int num_proc, rank;
	double* a = 0;
	double* sum = 0;
	double* c4 = 0;
	uint64_t l = 0, u = 0;

	doitgen_kernel_mpi_init(nr, nq, np, &num_proc, &rank, &a, &sum, &c4, &l, &u);
	MPI_Offset offset;
	MPI_File file;
	MPI_File_open(MPI_COMM_WORLD, output_path, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);

	flush_cache_epyc();
	MPI_Barrier(MPI_COMM_WORLD);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();


	for (uint64_t i = l; i < u; i++) {
		LSB_Res();
		offset = nq * np * sizeof(double) * i;
		MPI_File_write_at_all(file, offset, a, nq * np, MPI_DOUBLE, MPI_STATUS_IGNORE);
		LSB_Rec(0);
	}


	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	uint64_t elapsed = get_elapsed_us(start, end);

	mpi_clean_up(&file, a, sum, c4);
	return elapsed;
}


uint64_t kernel_doitgen_mpi_write_3(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path) {

	int num_proc, rank;
	double* a = 0;
	double* sum = 0;
	double* c4 = 0;
	uint64_t l = 0, u = 0;

	doitgen_kernel_mpi_init(nr, nq, np, &num_proc, &rank, &a, &sum, &c4, &l, &u);

	MPI_File file;
	mpi_io_init_file(nq, np, output_path, &file, l, u);

	flush_cache_epyc();
	MPI_Barrier(MPI_COMM_WORLD);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	for (uint64_t i = l; i < u; i++) {
		LSB_Res();
		MPI_File_write(file, a, np * nq, MPI_DOUBLE, MPI_STATUS_IGNORE);
		LSB_Rec(0);
	}


	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	uint64_t elapsed = get_elapsed_us(start, end);

	mpi_clean_up(&file, a, sum, c4);
	return elapsed;
}

uint64_t kernel_doitgen_mpi_write_4(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path) {

	int num_proc, rank;
	double* a = 0;
	double* sum = 0;
	double* c4 = 0;
	uint64_t l = 0, u = 0;

	doitgen_kernel_mpi_init(nr, nq, np, &num_proc, &rank, &a, &sum, &c4, &l, &u);

	MPI_File file;
	mpi_io_init_file(nq, np, output_path, &file, l, u);

	flush_cache_epyc();
	MPI_Barrier(MPI_COMM_WORLD);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	for (uint64_t i = l; i < u; i++) {
		LSB_Res();
		MPI_File_write_all(file, a, np * nq, MPI_DOUBLE, MPI_STATUS_IGNORE);
		LSB_Rec(0);
	}


	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	uint64_t elapsed = get_elapsed_us(start, end);

	mpi_clean_up(&file, a, sum, c4);
	return elapsed;
}

#define ARR_3D_UTILS(ARRAY, X_DIM, Y_DIM, Z_DIM, X, Y, Z) \
	(ARRAY[ ((Z_DIM) * (Y_DIM) * (X)) + ((Z_DIM) * (Y)) + (Z) ])

std::string print_array3D2(double* arr, uint64_t nr, uint64_t nq, uint64_t np) {
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

//https://pages.tacc.utexas.edu/~eijkhout/pcse/html/mpi-io.html
//https://cvw.cac.cornell.edu/parallelio/fileviewex
uint64_t kernel_doitgen_mpi_io(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path) {

	const uint64_t slices_per_batch = 64;
	int num_proc, rank;
	double* a = 0;
	double* sum = 0;
	double* c4 = 0;
	uint64_t l = 0, u = 0;

	doitgen_kernel_mpi_init(nr, nq, np, &num_proc, &rank, &a, &sum, &c4, &l, &u, slices_per_batch);

	uint64_t r = 0, q = 0, p = 0, s = 0;
	MPI_File file;
	mpi_io_init_file(nq, np, output_path, &file, l, u);

	flush_cache_epyc();
	MPI_Barrier(MPI_COMM_WORLD);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	for (r = l; r < u; r += slices_per_batch) {

		// - 2.1 init slice of A

		LSB_Res();
		init_A_slice_batch(nq, np, a, r, r + slices_per_batch);
		//std::cout << print_array3D2(a, slices_per_batch, nq, np) << std::endl;
		LSB_Rec(0);



		for (uint64_t rb = 0; rb < slices_per_batch; rb++) {

			LSB_Res();
			// - 2.2 batch eexecute kernel on slice
			for (q = 0; q < nq; q++) {

				for (p = 0; p < np; p++) {
					sum[p] = 0;
					for (s = 0; s < np; s++) {
						sum[p] += ARR_3D(a, slices_per_batch, nq, np, rb, q, s) * C4(s, p);
						//ARR_3D(a, i_upper - i_lower, nq, np, i - i_lower, j, k)
						//sum[p] += A_SLICE(q, s) * C4(s, p); //a[q * np + p] * c4[s * np + p];
					}
				}

				for (p = 0; p < np; p++) {
					ARR_3D(a, slices_per_batch, nq, np, rb, q, p) = sum[p];
					//A_SLICE(q, p) = sum[p];
				}
			}

			LSB_Rec(1);

		}


		LSB_Res();
		// 2.3 write A to the result file

		MPI_File_write(file, a, slices_per_batch * np * nq, MPI_DOUBLE, MPI_STATUS_IGNORE);
		LSB_Rec(2);
	}

	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	uint64_t elapsed = get_elapsed_us(start, end);

	mpi_clean_up(&file, a, sum, c4);

	return elapsed;
}

//https://pages.tacc.utexas.edu/~eijkhout/pcse/html/mpi-io.html
//https://cvw.cac.cornell.edu/parallelio/fileviewex
uint64_t kernel_doitgen_mpi_io_transpose(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path) {

	const uint64_t slices_per_batch = 64;
	int num_proc, rank;
	double* a = 0;
	double* sum = 0;
	double* c4 = 0;
	uint64_t l = 0, u = 0;

	doitgen_kernel_mpi_init(nr, nq, np, &num_proc, &rank, &a, &sum, &c4, &l, &u, slices_per_batch);

	uint64_t r = 0, q = 0, p = 0, s = 0;
	MPI_File file;
	mpi_io_init_file(nq, np, output_path, &file, l, u);

	flush_cache_epyc();
	MPI_Barrier(MPI_COMM_WORLD);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	LSB_Res();
	tranpose_C4(np, c4);
	LSB_Rec(0);

	for (r = l; r < u; r += slices_per_batch) {

		// - 2.1 init slice of A

		LSB_Res();
		init_A_slice_batch(nq, np, a, r, r + slices_per_batch);
		LSB_Rec(0);

		for (uint64_t rb = 0; rb < slices_per_batch; rb++) {

			LSB_Res();
			// - 2.2 batch eexecute kernel on slice
			for (q = 0; q < nq; q++) {

				for (p = 0; p < np; p++) {
					sum[p] = 0;
					for (s = 0; s < np; s++) {
						sum[p] += ARR_3D(a, slices_per_batch, nq, np, rb, q, s) * C4(p, s);
					}
				}

				for (p = 0; p < np; p++) {
					ARR_3D(a, slices_per_batch, nq, np, rb, q, p) = sum[p];
				}
			}

			LSB_Rec(1);

		}


		LSB_Res();
		// 2.3 write A to the result file

		MPI_File_write(file, a, slices_per_batch * np * nq, MPI_DOUBLE, MPI_STATUS_IGNORE);

		LSB_Rec(2);
	}

	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	uint64_t elapsed = get_elapsed_us(start, end);

	mpi_clean_up(&file, a, sum, c4);
	return elapsed;
}


/*
Finds a mpi benchmark by name
*/
bool find_benchmark_kernel_by_name(const std::string& benchmark_kernel_name, mpi_kernel_func* f) {

	bool found_kernel = false;

	for (int i = 0; i < NUM_DOIGTEN_MPI_KERNELS; i++) {
		const mpi_benchmark& b = mpi_benchmarks_data[i];
		if (b.name == benchmark_kernel_name) {
			found_kernel = true;
			*f = b.func;
			break;
		}
	}
	return found_kernel;
}