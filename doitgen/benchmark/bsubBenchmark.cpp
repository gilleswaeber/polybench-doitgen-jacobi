#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <omp.h>
#include <liblsb.h>
#include <utils.hpp>


void flush_cache_512()
{
  unsigned long long cs = 512 * 1024 * 1024;
  double* flush = (double*) calloc (cs, sizeof(double));
  unsigned long long i;
  double tmp = 0.0;
  for (i = 0; i < cs; i++)
    tmp += flush[i];
  //This is to prevent compiler optimizations
  if (tmp > 10.0) {
	  std::cout << "Fail !" << std::endl;
  }
  free ((void*)flush);
}


void fill_a_rand(int64_t* a, uint64_t size) {
	for (uint64_t i = 0; i < size; ++i) {
		a[i] = rand();
	}
}

void fill_rand_max(uint64_t* arr, uint64_t size) {
	for (uint64_t i = 0; i < size; ++i) {
		arr[i] = rand() % size;
	}
}

void benchmark_seq() {

	const uint64_t table_size = 1024;
	const uint64_t iter = 128;
	const uint64_t max_mem = 1024 * 1024 * 1024 / sizeof(int64_t);
	double temp = 0.0;

	for (uint64_t cur_size = table_size; cur_size <= max_mem; cur_size = cur_size << 1) {
		int64_t* a = (int64_t*)malloc(cur_size * sizeof(int64_t));
		fill_a_rand(a, cur_size);
		flush_cache_512();
		LSB_Set_Rparam_long("table_size", cur_size);
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


void benchmark_rand() {

	const uint64_t table_size = 1024;
	const uint64_t iter = 128;
	const uint64_t max_mem = 1024 * 1024 * 1024 / sizeof(int64_t);
	double temp = 0.0;

	for (uint64_t cur_size = table_size; cur_size <= max_mem; cur_size = cur_size << 1) {

		int64_t* a = (int64_t*)malloc(cur_size * sizeof(int64_t));
		uint64_t* random_indices = (uint64_t*) malloc(cur_size * sizeof(uint64_t));
		fill_rand_max(random_indices, cur_size);
		fill_a_rand(a, cur_size);
		flush_cache_512();

		LSB_Set_Rparam_long("table_size", cur_size);
		LSB_Res();
		for (uint64_t r = 0; r < iter; ++r) {
			for (uint64_t i = 0; i < cur_size; ++i) {
				temp += a[random_indices[i]];
			}
		}
		LSB_Rec(0);
		free(a);
		free(random_indices);
	}

	//Attempt at preventing optimizations
	std::cout << "Temp : " << temp << std::endl;
	
}

int main(int argc, char** argv) {
	if (argc < 4) return -1;

	uint64_t id = strtoull(argv[1], NULL, 0);
	const std::string benchmark_type = argv[2];
	const std::string sub_type = argv[3];

	const std::string benchmark_name = "bsub_benchmark_" + benchmark_type + "_" + sub_type + "_" +  std::to_string(id);

	MPI::Init();
	LSB_Init(benchmark_name.c_str(), 0);

	LSB_Set_Rparam_string("benchmark_name", benchmark_type.c_str());
	LSB_Set_Rparam_string("submission_type", sub_type.c_str());

	if (benchmark_type == "sequential") {
		std::cout << "benchmark sequential" << std::endl;
		benchmark_seq();
	} else if (benchmark_type == "random") {
		std::cout << "benchmark random" << std::endl;
		benchmark_rand();
	} else {
		std::cout << "benchmark snot found" << std::endl;
		return -1;
	}
	
	LSB_Finalize();
	MPI::Finalize();
}