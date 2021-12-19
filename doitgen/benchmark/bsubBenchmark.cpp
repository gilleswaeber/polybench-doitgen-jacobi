#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <omp.h>
#include <liblsb.h>
#include <utils.hpp>

void fill_a_rand(int64_t* a, uint64_t size) {
	for (uint64_t i = 0; i < size; ++i) {
		a[i] = rand();
	}
}

void benchmark() {
	const uint64_t table_size = 1024;
	const uint64_t iter = 128;
	const uint64_t max_mem = 256 * 1024 * 1024;
	double temp = 0.0;
	for (uint64_t cur_size = table_size; cur_size <= max_mem; cur_size = cur_size << 1) {
		int64_t* a = (int64_t*)malloc(cur_size * sizeof(int64_t));
		fill_a_rand(a, cur_size);
		flush_cache();
		LSB_Set_Rparam_long("TABLE_SIZE", cur_size);
		LSB_Res();
		for (uint64_t r = 0; r < iter; ++r) {
			for (uint64_t i = 0; i < cur_size; ++i) {
				temp += a[i];
			}
		}
		LSB_Rec(0);
		free(a);
	}
	//Attempt at preventing optimizations
	std::cout << "Temp : " << temp << std::endl;
	
}

int main(int argc, char** argv) {
	MPI::Init();
	LSB_Init("bsub_benchmark", 0);
	benchmark();
	LSB_Finalize();
	MPI::Finalize();
}