#include <iostream>
#include <mpi.h>
#include <chrono>
#include <thread>
#include <math.h>
#include <string>
#include <cstring>
#include <cassert>

#include "liblsb.h"

#include "doitgen.hpp"
#include "utils.hpp"
#include "serializer.hpp"

const int RUN = 5;
const char* output_file_path = "a.out";

int main(int argc, char **argv) {
	
	char* output_path = argv[1];

	uint64_t nr;
	uint64_t nq;
	uint64_t np;
	int num_proc;
	int rank;
	mpi_kernel_func selected_kernel;
	
	mpi_lsb_benchmark_startup(argv, argc, &nr, &nq, &np, &output_path, &selected_kernel);

	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Barrier(MPI_COMM_WORLD);
	selected_kernel(nr, nq, np, output_path);
	MPI_Barrier(MPI_COMM_WORLD);

	MASTER_MESSAGE("finished execution");

	mpi_lsb_benchmark_finalize();
}
