#include <iostream>
#include <check.h>
#include <ctime>
#include <vector>
#include <utils/utils.hpp>
#include <chrono>
#include <mpi.h>

#include "jacobi_1D.hpp"

struct Case {
    int n;
    int time_steps;
};

START_TEST(test_jacobi)
{
    std::vector<Case> cases = {
            {1'000, 100},
            {10'000, 200},
            {100'000, 500},
            {500'000, 1'000},
            {1'000'000, 10'000}
    };

    for (const auto & c : cases) {
        int n = c.n;
        int time_steps = c.time_steps;

        std::cout << "Testing with n=" << n << ", tsteps=" << time_steps << "\n";

        // Sequential execution ============================================================
        std::vector<double> A(n);
        {
            init_array(n, A.data());
            flush_cache();
            auto begin = std::chrono::high_resolution_clock::now();
            kernel_jacobi_1d_imper(time_steps, n, A.data());
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "  Sequential time : " << time_spent << "ms" << std::endl;
        }

        // Parallel execution ============================================================
        std::vector<double> A_par(n);
        {
            init_array(n, A_par.data());
            flush_cache();
            auto begin = std::chrono::high_resolution_clock::now();
            parallel_jacobi_1d_imper(time_steps, n, A_par.data());
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "  Parallel time : " << time_spent << "ms" << std::endl;
        }

        // MPI (single) execution ========================================================
        std::vector<double> A_mpi(n);

        {
            init_array(n, A_mpi.data());
            flush_cache();
            auto begin = std::chrono::high_resolution_clock::now();
            jacobi_1d_imper_mpi(time_steps, n, A_mpi.data(), {0, 1, 1});
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "  MPI time : " << time_spent << "ms" << std::endl;
        }

        //(*A)[10] *= 2;
        std::cerr << "  Compare reference with parallel execution\n";
        bool result = compare_results(n, A.data(), A_par.data());
        ck_assert(result);
        std::cerr << "  Compare reference with MPI execution\n";
        result = compare_results(n, A.data(), A_mpi.data());
        ck_assert(result);
    }
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

int main()
{
	std::cout << "### Test Jacobi Implementations ###" << std::endl;

    MPI_Init(nullptr, nullptr);
	int number_failed;
	Suite* s;
	SRunner* sr;

	s = jacobi_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
    MPI_Finalize();
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
