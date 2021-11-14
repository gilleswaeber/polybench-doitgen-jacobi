#include <iostream>
#include <check.h>
#include <ctime>
#include <limits>
#include <polybench.hpp>

#include "jacobi_1D.hpp"

#define LARGE_DATASET

void compare_results(int n,
    DATA_TYPE POLYBENCH_1D(A, N, n),
    DATA_TYPE POLYBENCH_1D(A_par, N, n)) {

    for (int i = 1; i < _PB_N - 1; i++) {
        bool check = std::abs(A[i] - A_par[i]) < std::numeric_limits<long>::epsilon();
        ck_assert(check);
    }
}

START_TEST(test_jacobi)
{
	int n = N;
    int tsteps = TSTEPS;

    // Sequential execution ============================================================
    POLYBENCH_1D_ARRAY_DECL(A, DATA_TYPE, N, n);
    POLYBENCH_1D_ARRAY_DECL(B, DATA_TYPE, N, n);

    init_array(n, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(B));
    polybench_flush_cache();
    clock_t begin = clock();
    kernel_jacobi_1d_imper(tsteps, n, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(B));
    clock_t end = clock();
    double time_spent = (double)(end - begin);
    POLYBENCH_FREE_ARRAY(B);

    std::cout << "Sequential time : " << time_spent << std::endl;

    // Parallel execution ============================================================
    POLYBENCH_1D_ARRAY_DECL(A_par, DATA_TYPE, N, n);
    POLYBENCH_1D_ARRAY_DECL(B_par, DATA_TYPE, N, n);

    init_array(n, POLYBENCH_ARRAY(A_par), POLYBENCH_ARRAY(B_par));
    polybench_flush_cache();
    begin = clock();
    parallel_jacobi_1d_imper(tsteps, n, POLYBENCH_ARRAY(A_par), POLYBENCH_ARRAY(B_par));
    end = clock();
    time_spent = (double)(end - begin);
    POLYBENCH_FREE_ARRAY(B_par);

    std::cout << "Parallel time : " << time_spent << std::endl;

    compare_results(n, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(A_par));

    POLYBENCH_FREE_ARRAY(A);
    POLYBENCH_FREE_ARRAY(A_par);
}
END_TEST

Suite* jacobi_suite(void)
{
	Suite* s;
	TCase* tc_core;

	s = suite_create("Jacobi");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_jacobi);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void)
{

	std::cout << "### Test Jacobi ###" << std::endl;

	int number_failed;
	Suite* s;
	SRunner* sr;

	s = jacobi_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
