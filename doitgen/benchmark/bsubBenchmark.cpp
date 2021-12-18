#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <omp.h>
#include <liblsb.h>
#include <utils.hpp>

void benchmark() {
	const uint64_t table_size = 1024;
	const uint64_t iter = 128;
	const uint64_t max_mem = 256 * 1024 * 1024;
	double temp = 0.0;
	uint64_t cur_rec = 0;
	for (uint64_t cur_size = table_size; cur_size <= max_mem; cur_size = cur_size << 1) {
		double* a = (double*)malloc(cur_size * sizeof(double));
		flush_cache();
		LSB_Set_Rparam_long("TABLE_SIZE", cur_size);
		cur_rec = 0;
		for (uint64_t r = 0; r < iter; ++r) {
			LSB_Res();
			for (uint64_t i = 0; i < table_size; ++i) {
				temp += a[i];
			}
			LSB_Rec(cur_rec);
		}
		cur_rec++;
		free(a);
	}
	//Attempt at preventing optimizations
	std::cout << "Temp : " << temp << std::endl;
	
}

int main(int argc, char** argv) {
	if (argc < 2) return -1;

	uint64_t threads = strtoull(argv[1], NULL, 0);
	
	omp_set_num_threads(threads);


	LSB_Init("bsub_benchmark", 0);
	
	benchmark();
	LSB_Rec(0);
	LSB_Finalize();
	MPI::Finalize();

}