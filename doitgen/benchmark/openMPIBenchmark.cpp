#include <openmpi/mpi.h>
#include <chrono>
#include <thread>

#include "doitgen.hpp"
#include "utils.hpp"
#include "serializer.hpp"

#define PROCESS_MESSAGE(RANK, MESSAGE) std::cout << "(" << (RANK) << ") " << (MESSAGE) << std::endl;

//PROCESS_MESSAGE(rank_world, "selected");
//std::this_thread::sleep_for(std::chrono::milliseconds(3000));
//benchmark here

int main() {

	MPI::Init();

	uint64_t nr = 257;
	uint64_t nq = 257;
	uint64_t np = 257;

	double* a = 0; double* sum = 0; double* c4 = 0; //to be freed at the end
	MPI_Win shared_window = 0; //to be freed at the end

	int num_proc_world = 0;
	int rank_world = 0;

    //Get the total number of processes available for the work (in the world)
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc_world);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

	if (rank_world == 0) {
		PROCESS_MESSAGE(rank_world, std::string("num processes = ") + std::to_string(num_proc_world));
	}

	//init data for the benchmark
	kernel_doitgen_mpi_init(&shared_window, nr, nq, np, &a, &c4, &sum);

	MPI_Barrier(MPI_COMM_WORLD);

	// select all power of two cores for the benchmark
	for (int i = 1; i <= num_proc_world; i *= 2) {

		MPI_Comm bench_comm = 0;
		MPI_Comm_split(MPI_COMM_WORLD, rank_world < i, rank_world, &bench_comm);

		if (rank_world == 0) {
			//PROCESS_MESSAGE(rank_world, std::string("cores # = ") + std::to_string(i));
		}

		if (rank_world < i) {
			
			//flush the cache before the kernel execution
			if (rank_world == 0) {
				flush_cache();
			}

			MPI_Barrier(bench_comm);

			auto t1 = std::chrono::high_resolution_clock::now();
				kernel_doitgen_mpi(bench_comm, nr, nq, np, a, c4, sum);
			MPI_Barrier(bench_comm); //sync all selected processes
			auto t2 = std::chrono::high_resolution_clock::now();
			auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
			
			if (rank_world == 0) {
				PROCESS_MESSAGE(rank_world, std::to_string(i) + std::string(" : ") + std::to_string(ms_int.count()));
			}

		} else {
			//PROCESS_MESSAGE(rank_world, "Ho no :(");
		}
		
		MPI_Comm_free(&bench_comm);
		MPI_Barrier(MPI_COMM_WORLD); //sync all processes
	}


	kernel_doitgen_mpi_clean(&shared_window, &sum);

	PROCESS_MESSAGE(rank_world, "exiting");

	MPI::Finalize();
	return 0;
}

/**
 * @brief Proof of concept of selecting a waiting process
 * 
 * @return int 
 */
int test() {

	MPI::Init();

	int num_proc_world = 0;
	int rank_world = 0;

    //Get the total number of processes available for the work (in the world)
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc_world);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

	if (rank_world == 0) {
		PROCESS_MESSAGE(rank_world, std::string("num processes = ") + std::to_string(num_proc_world));
	}

	MPI_Barrier(MPI_COMM_WORLD);

	for (int i = 0; i < num_proc_world; i++) {

		MPI_Comm bench_comm = 0;
		MPI_Comm_split(MPI_COMM_WORLD, rank_world <= i, rank_world, &bench_comm);

		if (rank_world <= i) {
			PROCESS_MESSAGE(rank_world, "Bonjour, j'execute :) dans le comm");
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			MPI_Barrier(bench_comm); //sync all selected processes
		} else {
			PROCESS_MESSAGE(rank_world, "Ho no :(");
		}
		
		MPI_Barrier(MPI_COMM_WORLD); //sync all processes
	}

	PROCESS_MESSAGE(rank_world, "bye !");

	MPI::Finalize();
	return 0;
}