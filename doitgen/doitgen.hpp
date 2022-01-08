#pragma once

#include <stdint.h>
#include <mpi.h>

#include <string>
#include <ctime>
#include <ratio>
#include <chrono>

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


const static problem_size_t problem_sizes[] = {
	{1, 10, 10},
	{10, 10, 10},
	{32, 32, 32},
	{64, 64, 64},
	{10, 5, 5},
	{64, 128, 128},
	{256, 512, 512},
	{512, 512, 512}
	//{256, 256, 256},
	//{257, 257, 257}//,
	//{128, 512, 512},
	//{512, 512, 512}
};

const static uint64_t PROBLEM_SIZE_N = sizeof(problem_sizes) / sizeof(problem_sizes[0]);;

const static problem_size_t benchmark_size = {128, 512, 512};




//https://cs61.seas.harvard.edu/wiki/images/0/0f/Lec14-Cache_measurement.pdf



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

/////////////////////////////////////// MPI kernels /////////////////
uint64_t kernel_doitgen_mpi_io(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path);
uint64_t kernel_doitgen_mpi_io_transpose(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path);

uint64_t kernel_doitgen_mpi_io_batch(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path);
uint64_t kernel_doitgen_mpi_io_transpose_batch(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path);

uint64_t kernel_doitgen_mpi_write_1(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path);
uint64_t kernel_doitgen_mpi_write_2(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path);
uint64_t kernel_doitgen_mpi_write_3(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path);
uint64_t kernel_doitgen_mpi_write_4(uint64_t nr, uint64_t nq, uint64_t np, const char* output_path);


//////////////////////////////////// MPI UTILS /////////////////////

std::string get_overall_file_name(char** argv, uint64_t num_processor);

uint64_t get_elapsed_us(std::chrono::high_resolution_clock::time_point& start, std::chrono::high_resolution_clock::time_point& end);

typedef uint64_t (*mpi_kernel_func)(uint64_t nr, uint64_t nq, uint64_t np, const char *output_path);

void mpi_lsb_benchmark_startup(char **argv, int argc, uint64_t* nr, uint64_t* nq, uint64_t* np, char** output_path, mpi_kernel_func* selected_kernel); 
void mpi_lsb_benchmark_finalize();

std::string get_benchmark_lsb_name(const std::string& benchmark_name, const std::string& processor_model, int num_processors);

bool find_benchmark_kernel_by_name(const std::string& benchmark_kernel_name, mpi_kernel_func* f);

struct mpi_benchmark {
	std::string name;
	mpi_kernel_func func;
};

void mpi_write_overall(const std::string& file_name, const std::string& benchmark_name, uint64_t run_index, uint64_t elapsed);

#define NUM_DOIGTEN_MPI_KERNELS 11
static const mpi_benchmark mpi_benchmarks_data[] = {

	{ "basic", &kernel_doitgen_mpi_io },
	{ "transpose", &kernel_doitgen_mpi_io_transpose },
	{ "transpose_1_per_node", &kernel_doitgen_mpi_io_transpose },
	{ "transpose_4_per_node", &kernel_doitgen_mpi_io_transpose },
	{ "transpose_12_per_node", &kernel_doitgen_mpi_io_transpose },
	{ "transpose_24_per_node", &kernel_doitgen_mpi_io_transpose },
	{ "transpose_48_per_node", &kernel_doitgen_mpi_io_transpose },
	{ "write_1", &kernel_doitgen_mpi_write_1 },
	{ "write_2", &kernel_doitgen_mpi_write_2 },
	{ "write_3", &kernel_doitgen_mpi_write_3 },
	{ "write_4", &kernel_doitgen_mpi_write_4 }

};

//////////////////////////////////////////////////////////


void kernel_doitgen_seq(uint64_t nr, uint64_t nq, uint64_t np, double* a, double* c4, double* sum);
void kernel_doitgen_polybench_parallel(uint64_t nr, uint64_t nq, uint64_t np, double* a, double* c4, double* sum);
void kernel_doitgen_polybench_parallel_local_sum(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
);

void kernel_doitgen_transpose(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4
);

void kernel_doitgen_transpose_local_sum(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* sum,
	double* c4
);

void kernel_doitgen_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	uint64_t blocking_window
);

void kernel_doitgen_inverted_loop(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4
);

void kernel_doitgen_inverted_loop_local_sum(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* sum,
	double* c4
);

void kernel_doitgen_inverted_loop_local_sum_1D(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* sum,
	double* c4
);

void kernel_doitgen_inverted_loop_blocking(uint64_t nr, uint64_t nq, uint64_t np, double* a_in,
	double* a_out, double* c4, uint64_t blocking_size);

void kernel_doitgen_inverted_loop_avx2(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4
);

void kernel_doitgen_inverted_loop_avx2_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	uint64_t blocking_window
);

void kernel_doitgen_inverted_loop_avx2_local_sum(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* sum,
	double* c4
);

void kernel_doitgen_inverted_loop_avx2_local_sum_1D(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* sum,
	double* c4
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

void kernel_doitgen_bikj_block_mapping(uint64_t nr, uint64_t nq, uint64_t np, double* a_in,
	double* a_out, double* c4, uint64_t blocking_size);

void kernel_doitgen_bikj_prime(uint64_t nr, uint64_t nq, uint64_t np, double* a_in,
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

//#endif /* !DOITGEN */
