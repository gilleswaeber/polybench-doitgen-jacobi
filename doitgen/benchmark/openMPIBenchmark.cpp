#include <mpi.h>
#include <chrono>
#include <thread>
#include <math.h>

#include "liblsb.h"

#include "doitgen.hpp"
#include "utils.hpp"
#include "serializer.hpp"

#define PROCESS_MESSAGE(RANK, MESSAGE) std::cout << "(" << (RANK) << ") " << (MESSAGE) << std::endl;

//PROCESS_MESSAGE(rank_world, "selected");
//std::this_thread::sleep_for(std::chrono::milliseconds(3000));
//benchmark here

const int num_cores_per_bench = 7;
const int cores[] = { 1, 2, 4, 8, 16, 32, 48 };

const int RUN = 10;

int main() {

	MPI::Init();

	uint64_t nr = benchmark_size.nr;
	uint64_t nq = benchmark_size.nq;
	uint64_t np = benchmark_size.np;

	double* a = 0; double* sum = 0; double* c4 = 0; //to be freed at the end
	MPI_Win shared_window = 0; //to be freed at the end

	int num_proc_world = 0;
	int rank_world = 0;

    //Get the total number of processes available for the work (in the world)
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc_world);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

	if (rank_world == 0) {
		PROCESS_MESSAGE(rank_world, std::string("\nnum processes = ") + std::to_string(num_proc_world));
	}

	LSB_Init("doitgen_mpi_benchmark", 0);
	
	
	//LSB_Set_Rparam_int("rank", rank_world);

	LSB_Set_Rparam_string("benchmark_type", "simple");

	LSB_Set_Rparam_long("nr", nr);
	LSB_Set_Rparam_long("nq", nq);
	LSB_Set_Rparam_long("np", np);

	//init data for the benchmark
	kernel_doitgen_mpi_init(&shared_window, nr, nq, np, &a, &c4, &sum);

	MPI_Barrier(MPI_COMM_WORLD);

	// select all power of two cores for the benchmark
	for (int i = 0; i < num_cores_per_bench; i++) {
		
		int num_current_cores = cores[i];

		if (num_current_cores > num_proc_world) {
			break;
		}

		LSB_Set_Rparam_int("rank", rank_world);
		LSB_Set_Rparam_int("num_cores", num_current_cores);
		for (int j = 0; j < RUN; j++) { // do the benchmark RUN times per process

			MPI_Comm bench_comm = 0;
			MPI_Comm_split(MPI_COMM_WORLD, rank_world < num_current_cores, rank_world, &bench_comm);

			if (rank_world == 0) {
				//PROCESS_MESSAGE(rank_world, std::string("cores # = ") + std::to_string(i));
			}

			if (rank_world < num_current_cores) {
				
				//flush the cache before the kernel execution
				if (rank_world == 0) {
					//flush_cache();
					init_array(nr, nq, np, a, c4);
					
				}

				memset(sum, 0, np);
				flush_cache();

				PROCESS_MESSAGE(rank_world, std::string("executing the kernel for # cores = ") + std::to_string(num_current_cores))

				MPI_Barrier(bench_comm);
				LSB_Res();
		
				kernel_doitgen_mpi(bench_comm, nr, nq, np, a, c4, sum);
				
				LSB_Rec(i);
				MPI_Barrier(bench_comm); //sync all selected processes

			} else {
				PROCESS_MESSAGE(rank_world, "sleeping...");
			}

			MPI_Comm_free(&bench_comm);
			MPI_Barrier(MPI_COMM_WORLD); //sync all processes

		}
	}

	kernel_doitgen_mpi_clean(&shared_window, &sum);

	PROCESS_MESSAGE(rank_world, "exiting");

	LSB_Finalize();
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