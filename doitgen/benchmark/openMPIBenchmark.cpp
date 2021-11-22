#include <openmpi/mpi.h>
#include <chrono>
#include <thread>

#include "doitgen.hpp"
#include "utils.hpp"
#include "serializer.hpp"

#define PROCESS_MESSAGE(RANK, MESSAGE) std::cout << "(" << (RANK) << ") " << (MESSAGE) << std::endl;

/**
 * @brief Proof of concept of selecting a waiting process
 * 
 * @return int 
 */
int main() {

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
			//MPI_Comm_split(MPI_COMM_WORLD, 0, rank_world, &bench_comm);
			PROCESS_MESSAGE(rank_world, "Bonjour, j'execute :) dans le comm");
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			MPI_Barrier(bench_comm);
			//MPI_Comm_free(&bench_comm);
		} else {
			//MPI_Comm_split(MPI_COMM_WORLD, 0, rank_world, &bench_comm);
			PROCESS_MESSAGE(rank_world, "Ho no :(");
			//MPI_Barrier(bench_comm);
		}
		
		MPI_Barrier(MPI_COMM_WORLD);
	}

	PROCESS_MESSAGE(rank_world, "bye !");

	MPI::Finalize();
	return 0;
}