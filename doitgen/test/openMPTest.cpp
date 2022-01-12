#include <iostream>
#include <check.h>
#include <limits>
#include <omp.h>
#include <chrono>
#include <string.h>
#include <openmpi/mpi.h>

#include "doitgen.hpp"
#include "utils.hpp"
#include "serializer.hpp"

#define THREAD_NUM 8

/*
* After thinking a bit about it, I'm in favor of dropping everything
* polybench related and only keeping the implementation.
*/

bool compare_results(uint64_t nr, uint64_t nq, uint64_t np, double* a, double* a_par) {
	bool result = true;
	for (uint64_t r = 0; r < nr; ++r) {
		for (uint64_t q = 0; q < nq; ++q) {
			for (uint64_t p = 0; p < np; ++p) {
				//bool test = std::abs(A[r][q][p] - A_par[r][q][p]) < std::numeric_limits<double>::epsilon();
				bool test = std::abs(ARR_3D(a, nr, nq, np, r, q, p) - ARR_3D(a_par, nr, nq, np, r, q, p)) < std::numeric_limits<double>::epsilon();
				if (!test) {
					result = false;
				}
			}
		}
	}
	return result;
}

void copy_array(double* a_in, double* a_out, uint64_t nr, uint64_t nq, uint64_t np) {
	for (uint64_t r = 0; r < nr; ++r) {
		for (uint64_t q = 0; q < nq; ++q) {
			for (uint64_t p = 0; p < np; ++p) {
				A_OUT(r, q, p) = A_IN(r, q, p);
			}
		}
	}
}

//https://stackoverflow.com/questions/16737298/what-is-the-fastest-way-to-transpose-a-matrix-in-c
void transpose(double* src, double* dst, uint64_t N, const int M) {
	#pragma omp parallel for
	for (uint64_t n = 0; n < N * M; n++) {
		uint64_t i = n / N;
		uint64_t j = n % N;
		dst[n] = src[M * j + i];
	}
}




START_TEST(test_doitgen)
{
	uint64_t nr = 512;
	uint64_t nq = 512;
	uint64_t np = 512;

	double* a_test = (double*) allocate_data(nr * nq * np, sizeof(double));

	loadFile("doitgen_dataset_512_512_512", nr * nq * np, a_test);

	double* a_in 	= (double*) allocate_data(nr * nq * np, sizeof(double));
	double* sum = (double*) allocate_data(nq * np * omp_get_max_threads(), sizeof(double));
	double* c4 	= (double*) allocate_data(np * np, sizeof(double));
	//double* c4_transposed = (double*)allocate_data(np * np, sizeof(double));

	//double* a_out = (double*)allocate_data(nr * nq * np, sizeof(double));

	init_array(nr, nq, np, a_in, c4);
	//copy_array(a_in, a_out, nr, nq, np);

	memset(sum, 0, nq * np * omp_get_max_threads() * sizeof(double));
	//transpose(c4, c4_transposed, np, np);
	//memset(a_out, 0.0, nr * np * nq * sizeof(double));
	
	flush_cache_openMP();
	auto t1 = std::chrono::high_resolution_clock::now();
	kernel_doitgen_inverted_loop_avx2_blocking_local_sum_2D(nr, nq, np, a_in, sum, c4, 32);
	auto t2 = std::chrono::high_resolution_clock::now();
	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

	std::cout << "Parallel time : " << ms_int.count() << std::endl;

	bool result = compare_results(nr, nq, np, a_in, a_test);
	ck_assert_msg(result, "results must match!");

	cleanup(a_test);
	cleanup(a_in);
	//cleanup(sum);
	cleanup(c4);
	//cleanup(c4_transposed);
	//cleanup(a_out);

}
END_TEST


Suite* doitgen_suite(void)
{
	Suite* s;
	TCase* tc_core;
	s = suite_create("Doitgen");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_doitgen);

	tcase_set_timeout(tc_core, 30);
	suite_add_tcase(s, tc_core);

	return s;
}


int main(void)
{
	
	omp_set_num_threads(THREAD_NUM);

	std::cout << "### Test Doitgen ###" << std::endl;
	int number_failed;
	Suite* s;
	SRunner* sr;

	s = doitgen_suite();
	sr = srunner_create(s);

	//This is to ease debugging
	srunner_set_fork_status(sr, CK_NOFORK);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	std::cout << "Press any key to continue..." << std::endl;
	getchar();

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
