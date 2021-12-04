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


//PROCESS_MESSAGE(rank_world, "selected");
//std::this_thread::sleep_for(std::chrono::milliseconds(3000));
//benchmark here


const int RUN = 5;

const char* output_file_path = "a.out";

int main(int argc, char **argv) {
	
	MPI_Init(nullptr, nullptr);

	//std::cout << argc << std::endl;

	assert(argc == 5);

	char* output_path = argv[1];

	remove(output_path);

	uint64_t nr = strtoull(argv[2], nullptr, 10);
	uint64_t nq = strtoull(argv[3], nullptr, 10);
	uint64_t np = strtoull(argv[4], nullptr, 10);

	int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	LSB_Init((std::string("doitgen_") + std::to_string(num_proc)).c_str(), 0);

	LSB_Set_Rparam_long("NR", nr);
	LSB_Set_Rparam_long("NQ", nq);
	LSB_Set_Rparam_long("NP", np);
	LSB_Set_Rparam_long("num_processes", num_proc);

	if (rank == 0) {
		std::cout << "starting benchmark without file" << nr << "x" << nq << "x" << np << std::endl;
		std::cout << "num process = " << num_proc << std::endl;
	}

	MPI_Barrier(MPI_COMM_WORLD);

	kernel_doitgen_mpi_io(nr, nq, np, output_path);
	
	MPI_Barrier(MPI_COMM_WORLD);

	MASTER_MESSAGE("finished execution");
	//remove(output_path);
	
	LSB_Finalize();
	MPI_Finalize();
}

/*
int main() {

	MPI_Init(nullptr, nullptr);

	// 0 - Init

	uint64_t nr = 128;
	uint64_t nq = 32;//512;
	uint64_t np = 32;//512;

	int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		PROCESS_MESSAGE_2(rank, "starting with #process = ", num_proc);
	}

	double* a = 0;
	double* sum = 0;
	double* c4 = 0;

	a = (double*) calloc(nq * np, sizeof(double));
	sum = (double*) calloc(np, sizeof(double));
	c4 = (double*) calloc(np * np, sizeof(double));

	assert(a != nullptr);
	assert(sum != nullptr);
	assert(c4 != nullptr);

	init_C4(np, c4);

	// 1 - split the job

	uint64_t chunk_size = nr / num_proc;
	uint64_t leftover = nr % num_proc; // we compute the imbalance in jobs
	uint64_t normal = num_proc - leftover; // the amount of processes that will not have an additional job
	uint64_t imbalanced_start = normal * chunk_size; //start index of the increased jobs

	uint64_t l = 0;
	uint64_t u = 0;
	
	if ((uint64_t)rank < normal) {
		l = rank * chunk_size;
		u = (rank + 1) * chunk_size;
	} else { // imbalanced workload process
		l = (rank - normal) * (chunk_size + 1) + imbalanced_start;
		u = (rank - normal + 1) * (chunk_size + 1) + imbalanced_start;
	}

	uint64_t r = 0, q = 0, p = 0, s = 0;
	MPI_Offset offset;

	// 2 - each do its job

	for (r = l; r < u; r++) {

		// - 2.1 init slice of A

		init_A_slice(nq, np, a, r);

		// - 2.2 execute kernel on slice
		for (q = 0; q < nq; q++) {

			for (p = 0; p < np; p++) {
				sum[p] = 0;
				for (s = 0; s < np; s++) {
					sum[p] += A_SLICE(q, s) * C4(s, p);
				}
			}

			for (p = 0; p < np; p++) {
				A_SLICE(q, p) = sum[p];
			}
		}

		// 2.3 write A to the result file

		offset = nq * np * sizeof(double) * r;

		MPI_File file;
		MPI_File_open(MPI_COMM_WORLD, output_file_path, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file); 
		MPI_File_write_at(file, offset, a, nq * np, MPI_DOUBLE, MPI_STATUS_IGNORE);
		
		MPI_File_close(&file);

	}

	//job finished we can exit

	free(a);
	free(sum);
	free(c4);

	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0) {
		PROCESS_MESSAGE(rank, "exiting!");
	}

	MPI_Finalize();

	return 0;
}*/