#pragma once

#include <stdint.h>
#include <mpi.h>

#include <string>

struct problem_size_t {
	uint64_t nr;
	uint64_t nq;
	uint64_t np; 
};

struct problem_instance_t {
	void (*kernel_id)(uint64_t, uint64_t, uint64_t, double*, double*, double*);
	std::string desc;
};

const static uint64_t num_processor = 8;
const static uint64_t PROBLEM_SIZE_N = 5;

const static problem_size_t problem_sizes[] = {
	{10, 10, 10},
	{32, 32, 32},
	{64, 64, 64},
	{256, 256, 256},
	//{257, 257, 257}//,
	{512, 512, 512}
};

const static problem_size_t benchmark_size = {128, 512, 512};

#define PROCESS_MESSAGE(RANK, MESSAGE) std::cout << "(" << (RANK) << ") " << (MESSAGE) << std::endl;
#define PROCESS_MESSAGE_2(RANK, MESSAGE_1, MESSAGE_2) std::cout << "(" << (RANK) << ") " << (MESSAGE_1) << (MESSAGE_2) << std::endl;
#define MASTER_MESSAGE_2(MESSAGE_1, MESSAGE_2) \
	if (rank == 0) { PROCESS_MESSAGE_2(rank, MESSAGE_1, MESSAGE_2); }

#define MASTER_MESSAGE(MESSAGE) \
	if (rank == 0) {PROCESS_MESSAGE(rank, MESSAGE); }

#define ARR_2D(ARRAY, Y_DIM, X, Y) (ARRAY[ (X) * (Y_DIM) + (Y) ])

#define C4(X, Y) ARR_2D(c4, np, X, Y)

//WARNING is A nq * np ?
#define A_SLICE(X, Y) ARR_2D(a, np, X, Y)

#define ARR_3D(ARRAY, X_DIM, Y_DIM, Z_DIM, X, Y, Z) \
	(ARRAY[ ((Z_DIM) * (Y_DIM) * (X)) + ((Z_DIM) * (Y)) + (Z) ])

#define A(X, Y, Z) ARR_3D(a, nr, nq, np, X, Y, Z)
#define A_IN(X, Y, Z) ARR_3D(a_in, nr, nq, np, X, Y, Z)
#define A_OUT(X, Y, Z) ARR_3D(a_out, nr, nq, np, X, Y, Z)

#define SUM(X, Y, Z) ARR_3D(sum, nr, nq, np, X, Y, Z)

void init_array(uint64_t nr, uint64_t nq, uint64_t np, double* A, double* C4);
void init_C4(uint64_t np, double* c4);
void init_A_slice(uint64_t nq, uint64_t np, double* a, uint64_t i);

void delete_file_if_exists(const char* output_path);
void kernel_doitgen_mpi_io(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path);

void kernel_doitgen_seq(uint64_t nr, uint64_t nq, uint64_t np, double* a, double* c4, double* sum);
void kernel_doitgen_openmp(uint64_t nr, uint64_t nq, uint64_t np, double* a, double* c4, double* sum);
void kernel_doitgen_experimental(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
);
void kernel_doitgen_transpose(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
);

void kernel_doitgen_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum,
	uint64_t blocking_window
);

void kernel_doitgen_transpose_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
);

extern "C" __attribute__((optimize("no-tree-vectorize")))
void kernel_doitgen_no_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
);

void kernel_doitgen_mpi(MPI_Comm bench_comm, uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
);

void kernel_doitgen_mpi(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
);

extern "C" __attribute__((optimize("no-tree-vectorize")))
void kernel_doitgen_bikj(uint64_t nr, uint64_t nq, uint64_t np, double* a_in,
	double* a_out, double* c4, uint64_t blocking_size);

/**
 * @brief Initialize the memory used by each process, MPI_Init must have been called previously.
 * 
 * @param nr 
 * @param nq 
 * @param np 
 * @param a 
 * @param c4 
 * @param sum 
 */
void kernel_doitgen_mpi_init(MPI_Win* shared_window, uint64_t nr, uint64_t nq, uint64_t np, double** a, double** c4, double** sum);

/**
 * @brief Free non shared memory used by MPI. This function does not call MPI_Finalize.
 * Note: Util now, we believe free memory is freed by finallize but not sure? KEEP IN MIND
 * As far as I can tell, the shared memory is freed when calling MPI_Finalize(). Moroever,
 * MPI_Finalize() must be the last call.
 * @param shared_window 
 * @param nr 
 * @param nq 
 * @param np 
 * @param sum 
 */
void kernel_doitgen_mpi_clean(MPI_Win* shared_window, double** sum);

const static problem_instance_t kernels_to_benchmark[] = {
	{kernel_doitgen_seq, "reference kernel"},
	{kernel_doitgen_openmp, "openMp implementation"}
};

//#endif /* !DOITGEN */
