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

	assert(argc == 8);

	char* output_path = argv[1];
	std::string benchmark_name = argv[2];
	std::string processor_model = argv[3];
	
	mpi_kernel_func selected_kernel;
	bool found_kernel = find_benchmark_kernel_by_name(benchmark_name, &selected_kernel);
	assert(found_kernel);

	std::cout << "launching benchmark for " << benchmark_name << std::endl;

	remove(output_path);

	uint64_t run_index =  strtoull(argv[4], nullptr, 10);

	uint64_t nr = strtoull(argv[5], nullptr, 10);
	uint64_t nq = strtoull(argv[6], nullptr, 10);
	uint64_t np = strtoull(argv[7], nullptr, 10);

	int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	LSB_Init(get_benchmark_lsb_name(benchmark_name, processor_model, num_proc).c_str(), 0);

	LSB_Set_Rparam_long("NR", nr);
	LSB_Set_Rparam_long("NQ", nq);
	LSB_Set_Rparam_long("NP", np);
	LSB_Set_Rparam_long("num_processes", num_proc);
	LSB_Set_Rparam_long("run_index", run_index);

	LSB_Set_Rparam_string("benchmark_type", benchmark_name.c_str());
	LSB_Set_Rparam_string("processor_model", processor_model.c_str());

	if (rank == 0) {
		std::cout << "starting benchmark without file" << nr << "x" << nq << "x" << np << std::endl;
		std::cout << "num process = " << num_proc << std::endl;
	}

	MPI_Barrier(MPI_COMM_WORLD);
	selected_kernel(nr, nq, np, output_path);
	MPI_Barrier(MPI_COMM_WORLD);

	MASTER_MESSAGE("finished execution");
	
	LSB_Finalize();
	MPI_Finalize();
}
