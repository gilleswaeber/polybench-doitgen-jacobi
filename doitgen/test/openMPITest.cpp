#include <iostream>
#include <check.h>
#include <limits>
#include <omp.h>
#include <chrono>
#include <string.h>
#include <openmpi/mpi.h>

#include "doitgen.hpp"
#include "utils.hpp"
#include "serializer.hpp"


void allocate_all(uint64_t nr, uint64_t nq, uint64_t np,
double** a_test,
	double** a_in,
	double** sum,
	double** c4,
	double** a_out) {

	*a_test 	= (double*) allocate_data(nr * nq * np, sizeof(double));
	*a_in 		= (double*) allocate_data(nr * nq * np, sizeof(double));
	*sum 		= (double*) allocate_data(nr * nq * np, sizeof(double));
	*c4 		= (double*) allocate_data(np * np, sizeof(double));
	*a_out 		= (double*)allocate_data(nr * nq * np, sizeof(double));

}

void copy_array(double* a_in, double* a_out, uint64_t nr, uint64_t nq, uint64_t np) {
	for (uint64_t r = 0; r < nr; ++r) {
		for (uint64_t q = 0; q < nq; ++q) {
			for (uint64_t p = 0; p < np; ++p) {
				A_OUT(r, q, p) = A_IN(r, q, p);
			}
		}
	}
}

/**
 * @brief 
 * 
 * Launch with hyperthreading
 * mpirun --use-hwthread-cpus ./dphpc-doitgen-mpi-test
 * Launch with CPU cores as processes
 * mpirun ./dphpc-doitgen-mpi-test
 * 
 * @return int 
 */
int main()
{
	
	MPI::Init();

    int communicator_size = 0;
	int rank = 0;

    //Get the total number of processes available for the work
	MPI_Comm_size(MPI_COMM_WORLD, &communicator_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	uint64_t nr = 32;
	uint64_t nq = 32;
	uint64_t np = 32;

	double* a_test 	= 0;
	double* a_in 	= 0;
	double* sum 	= 0;
	double* c4 		= 0;
	double* a_out 	= 0;
	
	if (rank == 0) {
		std::cout << "Master cleaning up" << std::endl;
		allocate_all(nr, nq, np, &a_test, &a_in, &sum, &c4, &a_out);
		loadFile(TEST_DIRECTORY_PATH"/doitgen_dataset_32_32_32", nr * nq * np, a_test);
		init_array(nr, nq, np, a_in, c4);
		copy_array(a_in, a_out, nr, nq, np);
		memset(sum, 0.0, nr * nq * np);
		//flush_cache();
	}
	
	auto t1 = std::chrono::high_resolution_clock::now();
	
	// launch kernel
	kernel_doitgen_mpi(nr, nq, np, a_in, a_out, c4, sum);

	auto t2 = std::chrono::high_resolution_clock::now();
	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

	std::cout << "rank : " << rank << " Parallel time : " << ms_int.count() << std::endl;

	//bool result = compare_results(nr, nq, np, a_out, a_test);
	//ck_assert_msg(result, "results must match!");

	if (rank == 0) {
		std::cout << "Master cleaning up" << std::endl;
		cleanup(a_test);
		cleanup(a_in);
		cleanup(a_out);
		cleanup(c4);
		cleanup(sum);
	}

	//std::cout << std::endl;
	//std::cout << "num_proc: " << communicator_size << std::endl;
	//std::cout << "rank: " << process_rank << std::endl;
    MPI::Finalize();
    
	return 0;

}