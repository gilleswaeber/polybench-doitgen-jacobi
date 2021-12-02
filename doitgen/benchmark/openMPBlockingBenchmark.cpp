#include <doitgen.hpp>
#include <utils.hpp>
#include <chrono>
#include <omp.h>
#include <liblsb.h>

#define RUNS 10

void init(double** a_in, double** a_out, double** c4, double** sum, uint64_t nr, uint64_t nq, uint64_t np) {
	*a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	*sum = (double*)allocate_data(nr * nq * np, sizeof(double));
	*c4 = (double*)allocate_data(np * np, sizeof(double));
	*a_out = (double*)allocate_data(nr * nq * np, sizeof(double));

	init_array(nr, nq, np, *a_in, *c4);
	memset(*a_out, 0.0, nr * nq * np * sizeof(double));
}

#define BLOCKING_WINDOW_SIZES 6
const static uint64_t blocking_windows[] = {16, 32, 64, 128, 256, 512 };

void transpose(double* src, double* dst, uint64_t N, const int M) {
#pragma omp parallel for
	for (uint64_t n = 0; n < N * M; n++) {
		uint64_t i = n / N;
		uint64_t j = n % N;
		dst[n] = src[M * j + i];
	}
}

int main() {
	uint64_t nr = 512;
	uint64_t nq = 512;
	uint64_t np = 512;

	double* a_in;
	double* a_out;
	double* c4;
	double* sum;
	init(&a_in, &a_out, &c4, &sum, nr, nq, np);

	MPI::Init();
	omp_set_num_threads(48);

	LSB_Init("doitgen-openMP-blocking", 0);

	LSB_Set_Rparam_long("NR", nr);
	LSB_Set_Rparam_long("NQ", nq);
	LSB_Set_Rparam_long("NP", np);

	/*
	* Benchmark with the blocking version.
	*/
	LSB_Set_Rparam_string("benchmark", "doitgen-seq-blocking");

	for (uint64_t i = 0; i < BLOCKING_WINDOW_SIZES; ++i) {
		LSB_Set_Rparam_long("blocking_size", blocking_windows[i]);
		for (uint64_t j = 0; j < RUNS; ++j) {
			LSB_Res();
			kernel_doitgen_bikj(nr, nq, np, a_in, a_out, c4, blocking_windows[i]);
			LSB_Rec(j);
			memset(a_out, 0, nr * nq * np * sizeof(double));
			flush_cache();
		}
	}

	flush_cache();
	LSB_Set_Rparam_string("benchmark", "doitgen-seq-no-blocking");
	for (uint64_t i = 0; i < RUNS; ++i) {
		LSB_Res();
		kernel_doitgen_no_blocking(nr, nq, np, a_in, a_out, c4, sum);
		LSB_Rec(i);
		memset(a_out, 0, nr * nq * np * sizeof(double));
		flush_cache();
	}

	LSB_Finalize();
	MPI::Finalize();

	cleanup(a_in);
	cleanup(a_out);
	cleanup(c4);
	cleanup(sum);
	return 0;
}