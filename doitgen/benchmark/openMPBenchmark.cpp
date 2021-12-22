#include <iostream>
#include <cstring>
#include <doitgen.hpp>
#include <stdlib.h>
#include <omp.h>
#include <liblsb.h>
#include <utils.hpp>

void transpose(double* src, double* dst, uint64_t N, uint64_t M) {
#pragma omp parallel for
	for (uint64_t n = 0; n < N * M; n++) {
		uint64_t i = n / N;
		uint64_t j = n % N;
		dst[n] = src[M * j + i];
	}
}

void do_polybench(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a = (double*)allocate_data(nr * nq * np, sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));
	double* sum = (double*)allocate_data(np, sizeof(double));

	init_array(nr, nq, np, a, c4);
	
	flush_cache();

	LSB_Res();
	kernel_doitgen_seq(nr, nq, np, a, c4, sum);
	LSB_Rec(0);

	free(a);
	free(c4);
	free(sum);
}

void do_polybench_parallel(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a = (double*)allocate_data(nr * nq * np, sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));
	double* sum = (double*)allocate_data(np * omp_get_max_threads(), sizeof(double));

	memset(sum, 0, np * omp_get_max_threads() * sizeof(double));

	init_array(nr, nq, np, a, c4);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_polybench_parallel(nr, nq, np, a, c4, sum);
	LSB_Rec(0);

	free(a);
	free(c4);
	free(sum);
}

void do_polybench_parallel_local_sum(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a = (double*)allocate_data(nr * nq * np, sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));
	uint64_t sum_size = omp_get_max_threads() * np;
	double* sum = (double*)allocate_data(sum_size, sizeof(double));

	init_array(nr, nq, np, a, c4);
	memset(sum, 0, sum_size * sizeof(double));

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_polybench_parallel_local_sum(nr, nq, np, a, c4, sum);
	LSB_Rec(0);

	free(a);
	free(c4);
}

void do_transpose(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	double* a_out = (double*)allocate_data(nr * nq * np, sizeof(double));
	memset(a_out, 0, nr * nq * np * sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	double* c4_transposed = (double*)allocate_data(np * np, sizeof(double));

	init_array(nr, nq, np, a_in, c4);
	transpose(c4, c4_transposed, np, np);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_transpose(nr, nq, np, a_in, a_out, c4_transposed);
	LSB_Rec(0);


	free(a_in);
	free(a_out);
	free(c4);
	free(c4_transposed);
}

void do_transpose_local_sum(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	uint64_t sum_size = omp_get_max_threads() * np;
	double* sum = (double*)allocate_data(sum_size, sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	double* c4_transposed = (double*)allocate_data(np * np, sizeof(double));

	init_array(nr, nq, np, a_in, c4);
	transpose(c4, c4_transposed, np, np);
	memset(sum, 0, sum_size * sizeof(double));

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_transpose_local_sum(nr, nq, np, a_in, sum, c4_transposed);
	LSB_Rec(0);


	free(a_in);
	free(sum);
	free(c4);
	free(c4_transposed);
}

void do_blocking(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	double* a_out = (double*)allocate_data(nr * nq * np, sizeof(double));
	memset(a_out, 0, nr * nq * np * sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	init_array(nr, nq, np, a_in, c4);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_blocking(nr, nq, np, a_in, a_out, c4, blocking_window);
	LSB_Rec(0);

	free(a_in);
	free(a_out);
	free(c4);
}

void do_inverted_loop(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	double* a_out = (double*)allocate_data(nr * nq * np, sizeof(double));
	memset(a_out, 0, nr * nq * np * sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	init_array(nr, nq, np, a_in, c4);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_inverted_loop(nr, nq, np, a_in, a_out, c4);
	LSB_Rec(0);

	free(a_in);
	free(a_out);
	free(c4);
}

void do_inverted_loop_local_sum(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	const uint64_t sum_size = omp_get_max_threads() * nq * np;
	double* sum = (double*)allocate_data(sum_size, sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	init_array(nr, nq, np, a_in, c4);
	memset(sum, 0, sum_size);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_inverted_loop_local_sum(nr, nq, np, a_in, sum, c4);
	LSB_Rec(0);

	free(a_in);
	free(sum);
	free(c4);
}

void do_inverted_loop_local_sum_1D(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	const uint64_t sum_size = omp_get_max_threads() * np;
	double* sum = (double*)allocate_data(sum_size, sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	init_array(nr, nq, np, a_in, c4);
	memset(sum, 0, sum_size);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_inverted_loop_local_sum_1D(nr, nq, np, a_in, sum, c4);
	LSB_Rec(0);

	free(a_in);
	free(sum);
	free(c4);
}

void do_inverted_loop_blocking(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	double* a_out = (double*)allocate_data(nr * nq * np, sizeof(double));
	memset(a_out, 0, nr * nq * np * sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	init_array(nr, nq, np, a_in, c4);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_inverted_loop_blocking(nr, nq, np, a_in, a_out, c4, blocking_window);
	LSB_Rec(0);

	free(a_in);
	free(a_out);
	free(c4);
}

void do_inverted_loop_avx2(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	double* a_out = (double*)allocate_data(nr * nq * np, sizeof(double));
	memset(a_out, 0, nr * nq * np * sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	init_array(nr, nq, np, a_in, c4);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_inverted_loop_avx2(nr, nq, np, a_in, a_out, c4);
	LSB_Rec(0);

	free(a_in);
	free(a_out);
	free(c4);
}

void do_inverted_loop_avx2_local_sum(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	uint64_t sum_size = omp_get_max_threads() * nq * np;
	double* sum = (double*)allocate_data(sum_size, sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	memset(sum, 0, sum_size * sizeof(double));

	init_array(nr, nq, np, a_in, c4);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_inverted_loop_avx2_local_sum(nr, nq, np, a_in, sum, c4);
	LSB_Rec(0);

	free(a_in);
	free(sum);
	free(c4);
}

void do_inverted_loop_avx2_local_sum_1D(uint64_t nr, uint64_t nq, uint64_t np, uint64_t blocking_window) {
	double* a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	uint64_t sum_size = omp_get_max_threads() * np;
	double* sum = (double*)allocate_data(sum_size, sizeof(double));
	double* c4 = (double*)allocate_data(np * np, sizeof(double));

	memset(sum, 0, sum_size * sizeof(double));

	init_array(nr, nq, np, a_in, c4);

	flush_cache_openMP();

	LSB_Res();
	kernel_doitgen_inverted_loop_avx2_local_sum_1D(nr, nq, np, a_in, sum, c4);
	LSB_Rec(0);

	free(a_in);
	free(sum);
	free(c4);
}


typedef void (*benchmark_func)(uint64_t, uint64_t, uint64_t, uint64_t);



struct Benchmark {
	const char* name;
	benchmark_func func;
};

//https://cs61.seas.harvard.edu/wiki/images/0/0f/Lec14-Cache_measurement.pdf
static const Benchmark benchmarks[] = {
	{"polybench", &do_polybench},
	{"polybench_parallel", &do_polybench_parallel},
	{"polybench_parallel_local_sum", &do_polybench_parallel_local_sum},
	{"transpose", &do_transpose},
	{"blocking", &do_blocking},
	{"inverted_loop", &do_inverted_loop},
	{"inverted_loop_local_sum", &do_inverted_loop_local_sum},
	{"inverted_loop_local_sum_1D", &do_inverted_loop_local_sum_1D},
	{"inverted_loop_blocking", &do_inverted_loop_blocking},
	{"inverted_loop_avx2", &do_inverted_loop_avx2},
	{"inverted_loop_avx2_local_sum", &do_inverted_loop_avx2_local_sum},
	{"inverted_loop_avx2_local_sum_1D", &do_inverted_loop_avx2_local_sum_1D},
	{"transpose_local_sum", &do_transpose_local_sum}
};

static const uint64_t benchmarks_size = sizeof(benchmarks) / sizeof(benchmarks[0]);

void help() {
	std::cout << "Usage : ./program benchmark_type nr nq np #threads run_id blocking_window(optional)" << '\n';
	std::cout << "benchmark_type: type of benchmark" << '\n';
	std::cout << "#threads: the number of threads to use" << '\n';
	std::cout << "nr nq np : problem size" << '\n';
	std::cout << "run_id : id of the current run" << '\n';
	std::cout << "blocking_window : optional argument to specify the blocking windows" << '\n';
}

int main(int argc, char** argv) {
	if (argc < 7) {
		std::cout << "Too few arguments..." << '\n';
		help();
		return -1;
	}
	if (argc > 8) {
		std::cout << "Too many arguments..." << '\n';
		help();
		return -1;
	}
	const char* benchmark_type = argv[1];
	uint64_t nr = strtoull(argv[2], NULL, 0);
	uint64_t nq = strtoull(argv[3], NULL, 0);
	uint64_t np = strtoull(argv[4], NULL, 0);

	uint64_t threads = strtoull(argv[5], NULL, 0);

	uint64_t run_id = strtoull(argv[6], NULL, 0);
	uint64_t blocking_window = 0;
	if (argc == 8) {
		blocking_window = strtoull(argv[7], NULL, 0);
	}

	omp_set_num_threads(threads);
	
	MPI::Init();

	const std::string benchmark_type_str = benchmark_type;
	const std::string benchmark_name = "doitgen-openMP-" + benchmark_type_str + "-" + 
		std::to_string(nr) + "-" + std::to_string(nq) + "-" + std::to_string(np) + "-" +
		std::to_string(threads) + "-" + std::to_string(blocking_window) + "-" + std::to_string(run_id);
	LSB_Init(benchmark_name.c_str(), 0);
	
	LSB_Set_Rparam_string("benchmark", benchmark_type);

	LSB_Set_Rparam_long("NR", nr);
	LSB_Set_Rparam_long("NQ", nq);
	LSB_Set_Rparam_long("NP", np);


	LSB_Set_Rparam_long("threads", threads);
	
	LSB_Set_Rparam_long("blocking_size", blocking_window);


	for (uint64_t i = 0; i < benchmarks_size; ++i) {
		if (strcmp(benchmark_type, benchmarks[i].name) == 0) {
			benchmarks[i].func(nr, nq, np, blocking_window);
			break;
		}
	}
	LSB_Finalize();
	MPI::Finalize();

}