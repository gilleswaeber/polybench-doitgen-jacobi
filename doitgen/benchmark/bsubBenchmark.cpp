#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <omp.h>
#include <liblsb.h>

void benchmark() {
	const uint64_t table_size = 1024 * 1024 * 128;
	const uint64_t iter = 128;
	double* a = (double*)malloc(table_size * sizeof(double));
	double temp = 0.0;

	for (uint64_t r = 0; r < iter; ++r)  {
		for (uint64_t i = 0; i < table_size; ++i) {
			temp += a[i];
		}
	}
	std::cout << "Temp : " << temp << std::endl;
	free(a);
}

int main(int argc, char** argv) {
	if (argc < 2) return -1;

	uint64_t threads = strtoull(argv[1], NULL, 0);
	
	omp_set_num_threads(threads);


	LSB_Init("bsub_benchmark", 0);
	LSB_Res();
	benchmark();
	LSB_Rec(0);
	LSB_Finalize();
	MPI::Finalize();

}