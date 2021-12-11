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

typedef void (*kernel_func)(uint64_t nr, uint64_t nq, uint64_t np, const char *output_path);

struct Benchmark {
	std::string name;
	kernel_func func;
};

//https://cs61.seas.harvard.edu/wiki/images/0/0f/Lec14-Cache_measurement.pdf
#define NUM_DOIGTEN_MPI_KERNELS 1
static const Benchmark benchmarks[] = {
	{ "basic", &kernel_doitgen_mpi_io }
};

const int RUN = 5;
const char* output_file_path = "a.out";

int main(int argc, char **argv) {
	
	MPI_Init(nullptr, nullptr);

	//std::cout << argc << std::endl;

	assert(argc == 6);

	char* output_path = argv[1];
	std::string benchmark_name = argv[2];
	
	kernel_func selected_kernel;
	bool found_kernel = false;

	for (int i = 0; i < NUM_DOIGTEN_MPI_KERNELS; i++) {
		const Benchmark& b = benchmarks[i];
		if (b.name == benchmark_name) {
			found_kernel = true;
			selected_kernel = b.func;
			break;
		}
	}

	assert(found_kernel);

	std::cout << "launching benchmark for " << benchmark_name << std::endl;

	remove(output_path);

	uint64_t nr = strtoull(argv[3], nullptr, 10);
	uint64_t nq = strtoull(argv[4], nullptr, 10);
	uint64_t np = strtoull(argv[5], nullptr, 10);

	int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	LSB_Init((std::string("doitgen_") + benchmark_name + std::string("_") + std::to_string(num_proc)).c_str(), 0);

	LSB_Set_Rparam_long("NR", nr);
	LSB_Set_Rparam_long("NQ", nq);
	LSB_Set_Rparam_long("NP", np);
	LSB_Set_Rparam_long("num_processes", num_proc);

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
