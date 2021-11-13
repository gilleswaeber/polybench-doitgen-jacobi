#include <iostream>
#include <check.h>
#include <limits>
#include <omp.h>
#include <chrono>

#include "serializer.hpp"

#define THREAD_NUM 16

/*
* After thinking a bit about it, I'm in favor of dropping everything
* polybench related and only keeping the implementation.
*/

bool compare_results(uint64_t nr, uint64_t nq, uint64_t np,
	DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np),
	DATA_TYPE POLYBENCH_3D(A_par, NR, NQ, NP, nr, nq, np)) {
	bool result = true;
	for (uint64_t r = 0; r < _PB_NR; ++r) {
		for (uint64_t q = 0; q < _PB_NQ; ++q) {
			for (uint64_t p = 0; p < _PB_NP; ++p) {
				bool test = std::abs(A[r][q][p] - A_par[r][q][p]) < std::numeric_limits<double>::epsilon();
				if (!test) {
					result = false;
				}
			}
		}
	}
	return result;
}

START_TEST(test_doitgen)
{
	uint64_t nr = NR;
	uint64_t nq = NQ;
	uint64_t np = NP;

	POLYBENCH_3D_ARRAY_DECL(A_test, DATA_TYPE, NR, NQ, NP, nr, nq, np);
	loadFile(nr, nq, np, POLYBENCH_ARRAY(A_test));

	POLYBENCH_3D_ARRAY_DECL(A, DATA_TYPE, NR, NQ, NP, nr, nq, np);
	POLYBENCH_3D_ARRAY_DECL(sum, DATA_TYPE, NR, NQ, NP, nr, nq, np);
	POLYBENCH_2D_ARRAY_DECL(C4, DATA_TYPE, NP, NP, np, np);

	polybench_flush_cache();

	auto t1 = std::chrono::high_resolution_clock::now();
	parallel_doitgen(nr, nq, np,
		POLYBENCH_ARRAY(A),
		POLYBENCH_ARRAY(C4),
		POLYBENCH_ARRAY(sum));
	auto t2 = std::chrono::high_resolution_clock::now();
	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

	std::cout << "Parallel time : " << ms_int.count() << std::endl;



	bool result = compare_results(nr, nq, np, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(A_test));
	ck_assert(true);

	POLYBENCH_FREE_ARRAY(A_test);

	POLYBENCH_FREE_ARRAY(A);
	POLYBENCH_FREE_ARRAY(C4);
	POLYBENCH_FREE_ARRAY(sum);

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

	/*uint64_t nr = NR;
	uint64_t nq = NQ;
	uint64_t np = NP;

	POLYBENCH_3D_ARRAY_DECL(A_test, DATA_TYPE, NR, NQ, NP, nr, nq, np);
	//loadFile(nr, nq, np, POLYBENCH_ARRAY(A_test));

	POLYBENCH_3D_ARRAY_DECL(A, DATA_TYPE, NR, NQ, NP, nr, nq, np);
	POLYBENCH_3D_ARRAY_DECL(sum, DATA_TYPE, NR, NQ, NP, nr, nq, np);
	POLYBENCH_2D_ARRAY_DECL(C4, DATA_TYPE, NP, NP, np, np);

	

	kernel_doitgen(nr, nq, np,
		POLYBENCH_ARRAY(A),
		POLYBENCH_ARRAY(C4),
		POLYBENCH_ARRAY(sum));

	writeFile(nr, nq, np, POLYBENCH_ARRAY(A));

	loadFile(nr, nq, np, POLYBENCH_ARRAY(A_test));
	bool result = compare_results(nr, nq, np, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(A_test));

	if (result) {
		std::cout << "Success" << std::endl;
	}
	else {
		std::cout << "Failure" << std::endl;
	}

	POLYBENCH_FREE_ARRAY(A);
	POLYBENCH_FREE_ARRAY(C4);
	POLYBENCH_FREE_ARRAY(sum);
	POLYBENCH_FREE_ARRAY(A_test);


	return 0;*/
}
