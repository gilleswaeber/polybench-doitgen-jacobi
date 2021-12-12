#include <iostream>
#include <check.h>
#include <limits>
#include <omp.h>
#include <chrono>
#include <string.h>
#include <openmpi/mpi.h>
#include <string>
#include <cassert>
#include <stdio.h>
#include <liblsb.h>

#include "doitgen.hpp"
#include "utils.hpp"
#include "serializer.hpp"

#define MPI_ERROR_CHECK(CALL) \
	if (CALL != MPI_SUCCESS) { \
		std::cout << "ERROR CODE : " << CALL << std::endl; \	
	} \

#define PROCESS_MESSAGE(RANK, MESSAGE) std::cout << "(" << (RANK) << ") " << (MESSAGE) << std::endl;

/**
 * Shared memory with MPI : https://pages.tacc.utexas.edu/~eijkhout/pcse/html/mpi-shared.html
 */

std::string get_dataset_path(uint64_t nr, uint64_t nq, uint64_t np) {
	std::string result = TEST_DIRECTORY_PATH"/doitgen_dataset_";
	result += std::to_string(nr); result += "_";
	result += std::to_string(nq); result += "_";
	result += std::to_string(np);
	return result;
}


void print_state(int world_rank, double* a, double* c4, double* sum) {
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "world id : " << world_rank << std::endl;
	std::cout << "--> a 	: " << (std::size_t) a << std::endl;
	std::cout << "--> c4 	: " << (std::size_t) c4 << std::endl;
	std::cout << "--> sum : " << (std::size_t) sum << std::endl;
}

bool compare_results(uint64_t nr, uint64_t nq, uint64_t np, double* expected, double* actual) {
	bool result = true;
	for (uint64_t r = 0; r < nr; ++r) {
		for (uint64_t q = 0; q < nq; ++q) {
			for (uint64_t p = 0; p < np; ++p) {
				//bool test = std::abs(A[r][q][p] - A_par[r][q][p]) < std::numeric_limits<double>::epsilon();
				bool test = std::abs(ARR_3D(expected, nr, nq, np, r, q, p) - ARR_3D(actual, nr, nq, np, r, q, p)) < std::numeric_limits<double>::epsilon();
				if (!test) {
					//std::cout << "ERR : exp = " << ARR_3D(a, nr, nq, np, r, q, p) << ", got = " << ARR_3D(a_par, nr, nq, np, r, q, p) << std::endl;
					result = false;
				}
			}
		}
	}

	if (result == false) {
		std::cout << "expected:" << std::endl;
		std::cout << print_array3D(expected, nr, nq, np) << std::endl;
		std::cout << "actual:" << std::endl;
		std::cout << print_array3D(actual, nr, nq, np) << std::endl;
	}

	return result;
}

int main(int argc, char **argv) {

	MPI_Init(nullptr, nullptr);

	//std::cout << argc << std::endl;

	assert(argc == 8);

	char* output_path = argv[1];
	std::string benchmark_name = argv[2];
	std::string processor_model = argv[3];

	remove(output_path);

	uint64_t run_index =  strtoull(argv[4], nullptr, 10);

	uint64_t nr = strtoull(argv[5], nullptr, 10);
	uint64_t nq = strtoull(argv[6], nullptr, 10);
	uint64_t np = strtoull(argv[7], nullptr, 10);

	int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	LSB_Init(get_benchmark_lsb_name(benchmark_name, processor_model, num_proc).c_str(), 0);

	LSB_Set_Rparam_string("benchmark_name", benchmark_name.c_str());
	LSB_Set_Rparam_string("processor_model", processor_model.c_str());

	LSB_Set_Rparam_long("NR", nr);
	LSB_Set_Rparam_long("NQ", nq);
	LSB_Set_Rparam_long("NP", np);
	LSB_Set_Rparam_long("num_processes", num_proc);
	LSB_Set_Rparam_long("run_index", run_index);

	mpi_kernel_func f;
	bool found_kernel = find_benchmark_kernel_by_name(benchmark_name, &f);
	assert(found_kernel);
	assert(f);

	f(nr, nq, np, output_path);
	
	//here we load the file and check its result if we are the master
	if (rank == 0) {

		double* expected = 0;
		expected = (double*) calloc(nr * nq * np, sizeof(double));
		assert(expected != 0);

		double* a = 0;
		a = (double*) calloc(nr * nq * np, sizeof(double));
		assert(a != 0);
	
		loadFile(get_dataset_path(nr, nq, np), nr * nq * np, expected);
		loadFile(output_path, nr * nq * np, a);

		int result = compare_results(nr, nq, np, expected, a);
		if (!result) {
			std::cout << "Test failed !" << std::endl;
			free(a);
			free(expected);
			assert(false);
		} else {
			std::cout << "Success !" << std::endl;
			free(a);
			free(expected);
		}

	}

	LSB_Finalize();
	MPI_Finalize();
}

/**
 * @brief 
 * 
 * doc : https://pages.tacc.utexas.edu/~eijkhout/pcse/html/index.html
 * shared : https://stackoverflow.com/questions/68369535/using-mpi-to-share-bulk-data-between-processes
 * Launch with hyperthreading
 * mpirun --use-hwthread-cpus ./dphpc-doitgen-mpi-test
 * Launch with CPU cores as processes
 * mpirun ./dphpc-doitgen-mpi-test
 * 
 * @return int 
 */
/*
int main()
{


	std::cout << "hello" << std::endl;
	
	MPI::Init();

	uint64_t nr = 32;
	uint64_t nq = 32;
	uint64_t np = 32;

	double* a_test = 0; double* a = 0; double* sum = 0; double* c4 = 0;

    int world_size = 0;
	int world_rank = 0;

	MPI_Win shared_window = 0;

    //Get the total number of processes available for the work (in the world)
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	MPI_Aint a_size = nr * nq * np * sizeof(double);

	//init memory for all the processes
	kernel_doitgen_mpi_init(&shared_window, nr, nq, np, &a, &c4, &sum);

	if (world_rank == 0) { //allocate the test array only on the master
		a_test = (double*) MPI::Alloc_mem(a_size, MPI_INFO_NULL);
		loadFile(get_dataset_path(nr, nq, np), nr * nq * np, a_test);
	}

	MPI_Barrier(MPI_COMM_WORLD); // Everyone begin at the same time 

	auto t1 = std::chrono::high_resolution_clock::now();
	
		// launch kernel
		PROCESS_MESSAGE(world_rank, "Proceeding to kernel execution");
		kernel_doitgen_mpi(nr, nq, np, a, c4, sum);
		PROCESS_MESSAGE(world_rank, "waiting for friends");
		MPI_Barrier(MPI_COMM_WORLD);
	
	auto t2 = std::chrono::high_resolution_clock::now();
	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

	if (world_rank == 0) {
		PROCESS_MESSAGE(world_rank, ms_int.count());
		bool result = compare_results(nr, nq, np, a, a_test);
		if (result) {
			PROCESS_MESSAGE(world_rank, "SUCCESS");
		} else {
			PROCESS_MESSAGE(world_rank, "FAILED");
		}
	}

	if (world_rank == 0) {
		MPI_Free_mem(a_test);
	}

	//MPI_Free_mem(sum);
	//MPI_Win_free(&shared_window);
	kernel_doitgen_mpi_clean(&shared_window, &sum);

    MPI::Finalize();
	
	PROCESS_MESSAGE(world_rank, "exiting :)");

	return 0;

}*/


/*
	PROCESS_MESSAGE(world_rank, "starting :)");

	MPI_Aint a_size = nr * nq * np * sizeof(double);
	MPI_Aint c4_size = np * np * sizeof(double);
	int displacement_unit = sizeof(double); //placeholder

	print_state(world_rank, a, c4, sum);

	if (world_rank == 0) {
      
		MPI_Win_allocate_shared(
			a_size, 
			sizeof(double),
			MPI_INFO_NULL,
			MPI_COMM_WORLD, 
			&a, 
			&shared_window
		);

		MPI_Win_allocate_shared(
			c4_size, 
			sizeof(double),
			MPI_INFO_NULL,
			MPI_COMM_WORLD, 
			&c4, 
			&shared_window
		);


	} else {

		MPI_Win_allocate_shared(
			0, 
		   	sizeof(double), 
		  	MPI_INFO_NULL,
        	MPI_COMM_WORLD,
			&a,
			&shared_window
		);

		MPI_Win_shared_query(shared_window, 0, &a_size, &displacement_unit, &a);

		displacement_unit = 0;
		MPI_Win_allocate_shared(
			0, 
		   	sizeof(double), 
		  	MPI_INFO_NULL,
        	MPI_COMM_WORLD,
			&c4,
			&shared_window
		);

		MPI_Win_shared_query(shared_window, 0, &c4_size, &displacement_unit, &c4);
	
	}

	MPI_Barrier(MPI_COMM_WORLD); // wait that all processes have requested thier memory

	if (world_rank == 0) { //allocate the test array only on the master
		a_test = (double*) MPI::Alloc_mem(a_size, MPI_INFO_NULL);
	}

	// sum is private too (everyone allocates its own)
	sum = (double*) MPI::Alloc_mem(np * sizeof(double), MPI_INFO_NULL);
	memset(sum, 0.0, np);

	print_state(world_rank, a, c4, sum);

	if (world_rank == 0) {
		PROCESS_MESSAGE(world_rank, "initialize test a_test and c4");
		loadFile(get_dataset_path(nr, nq, np), nr * nq * np, a_test);
		init_array(nr, nq, np, a, c4);
	}

	MPI_Barrier(MPI_COMM_WORLD); // wait that all threads have seen initialization of a and C4

	if (world_rank == 0 || world_rank == 1) {
		//PROCESS_MESSAGE(world_rank, print2D(c4, np, np));
		//PROCESS_MESSAGE(world_rank, print_array3D(a, nr, nq, np));
		if (a_test) {
			//PROCESS_MESSAGE(world_rank, print_array3D(a_test, nr, nq, np));
		}
	}
	*/

//double * mem = 0;
	//https://www.mpich.org/static/docs/v3.3/www3/MPI_Win_allocate.html
	/*MPI_Win_allocate(
		nr * nq * np + np * np, 
		sizeof(double), 
		MPI_INFO_NULL, 
		MPI_COMM_WORLD, 
		&mem, 
		&shared_window
	);*/


/*
int main()
{
	
	MPI::Init();

    int world_size = 0;
	int world_rank = 0;

	int local_size = 0;
	int local_rank = 0;

	MPI_Win shared_window;

    //Get the total number of processes available for the work (in the world)
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	// create local node communicator (https://stackoverflow.com/questions/39912588/can-i-use-mpi-with-shared-memory)
	MPI_Comm local_comm; // local communicator 
	
	//this allows to allocate memory on a single node and access from other
	//be aware of remote accesses !
	MPI_Comm_split_type(
		MPI_COMM_WORLD, 
		MPI_COMM_TYPE_SHARED,
		world_rank, 
		MPI_INFO_NULL, 
		&local_comm
	);

	MPI_Comm_size(local_comm, &local_size);
  	MPI_Comm_rank(local_comm, &local_rank);

	//Now two contexts exists, world and local (processes might have different rank in both)

	uint64_t nr = 32;
	uint64_t nq = 32;
	uint64_t np = 32;

	double* a_test 	= 0;
	double* a 		= 0;
	double* sum 	= 0;
	double* c4 		= 0;

	print_state(world_rank,local_rank, a, c4, sum);
	allocate_all(local_rank, local_comm, &shared_window, nr, nq, np, &a_test, &a, &sum, &c4);

	if (world_rank == 0) {
		std::cout << "Master initialize test_a and C4" << std::endl;
		loadFile(TEST_DIRECTORY_PATH"/doitgen_dataset_32_32_32", nr * nq * np, a_test);
		//init locally a and C4
		init_array(nr, nq, np, a, c4);
		//flush_cache();
	}

	// All processes initialize their private sum 
	memset(sum, 0.0, np);
	
	auto t1 = std::chrono::high_resolution_clock::now();
	
	// launch kernel
	//kernel_doitgen_mpi(nr, nq, np, a, c4, sum);

	auto t2 = std::chrono::high_resolution_clock::now();
	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

	//std::cout << "rank : " << process_rank << " Parallel time : " << ms_int.count() << std::endl;
	//print_state(process_rank, a, c4, sum, mem);

	//bool result = compare_results(nr, nq, np, a_out, a_test);
	//ck_assert_msg(result, "results must match!");

	// Is mem managed by mpi now ?
	if (world_rank == 0) {
		std::cout << "Master cleaning up" << std::endl;
		cleanup(a);
		cleanup(c4);
	}

	cleanup(sum);

	MPI_Win_free(&shared_window);
    MPI::Finalize();
    
	return 0;

}*/

/*
// from what I've read, only the master allocates the shared memory and initialize it.
void allocate_all(int local_rank, MPI_Comm local_comm, MPI_Win* shared_window, uint64_t nr, uint64_t nq, uint64_t np,
	double** a_test,
	double** a,
	double** sum,
	double** c4
) {
	
	std::size_t n = nr * nq * np * sizeof(double);
	

	MPI_ERROR_CHECK(
		MPI_Win_allocate_shared(
			local_rank == 0 ? nr * nq * np * sizeof(double) : 0, // 0 if a non-master process allocates
			sizeof(double),
			MPI_INFO_NULL, 
			local_comm,
			*a, 
			shared_window
		)
	);

	MPI_ERROR_CHECK(
		MPI_Win_allocate_shared(
			local_rank == 0 ? np * np * sizeof(double) : 0,
			sizeof(double),
			MPI_INFO_NULL, 
			local_comm,
			*c4, 
			shared_window
		)
	);

	if (local_rank == 0) { //allocate the test array only on the master
		*a_test = (double*) MPI::Alloc_mem(n, MPI_INFO_NULL);
	}

	// sum is private too (everyone allocates its own)
	*sum = (double*) MPI::Alloc_mem(np * sizeof(double), MPI_INFO_NULL);
	
}


void copy_array(double* a_in, double* a_out, uint64_t nr, uint64_t nq, uint64_t np) {
	for (uint64_t r = 0; r < nr; ++r) {
		for (uint64_t q = 0; q < nq; ++q) {
			for (uint64_t p = 0; p < np; ++p) {
				A_OUT(r, q, p) = A_IN(r, q, p);
			}
		}
	}
}*/