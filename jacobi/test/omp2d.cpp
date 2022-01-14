#include <iostream>
#include <check.h>
#include <vector>
#include <utils.hpp>
#include <chrono>
#include <mpi.h>

#include "jacobi2d_omp.hpp"

struct Case {
    int n;
    int time_steps;
};


#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
bool compare_results(int n, std::vector<double> *A, std::vector<double> *B) {
    constexpr int show_errors = 20;
    const double allowed_relative_error = 1e-12;

    long errors = 0;
    long close = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (A[i][j] != B[i][j]) {
                double a = std::min(A[i][j], B[i][j]), b = std::max(A[i][j], B[i][j]);
                if (!(a + std::abs(a) * allowed_relative_error > b)) { // ensure no match on NaNs
                    ++errors;
                    if (errors <= show_errors)
                        std::cerr << "  A[" << i << "][" << j << "] = " << A[i][j] << " ≠ B[" << i << "][" << j << "] = " << B[i][j] << "! (diff"
                                  << (A[i][j] - B[i][j]) << " )\n";
                } else ++close;
            }
        }
    }
    if (errors > show_errors) std::cerr << "    and " << (errors - show_errors) << " more\n";
    if (close) std::cerr << "  " << close << " within tolerance range (" << allowed_relative_error << ")\n";
    return errors == 0;
}
#pragma clang diagnostic pop

START_TEST(test_jacobi_2d)
{
    std::vector<Case> cases = {
            {32, 2},
            {500, 10},
            {1'000, 20},
            {2'000, 20},
            {4'000, 100}
    };

    for (const auto & c : cases) {
        int n = c.n;
        int time_steps = c.time_steps;

        std::cout << "Testing with n=" << n << ", tsteps=" << time_steps << "\n";

        // Sequential execution ============================================================
        std::vector<std::vector<double>> A(n, std::vector<double>(n));
        std::cout << A.size() << " " << A[0].size() << std::endl;
        {
            init_array(n, A.data());
            flush_cache();
            auto begin = std::chrono::high_resolution_clock::now();
            kernel_jacobi_2d_imper(time_steps, n, A.data());
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "  Sequential time : " << time_spent << "ms" << std::endl;
        }

        // Parallel execution ============================================================
        std::vector<std::vector<double>> A_par(n, std::vector<double>(n));
        {
            init_array(n, A_par.data());
            flush_cache();
            auto begin = std::chrono::high_resolution_clock::now();
            kernel_jacobi_2d_imper_par(time_steps, n, A_par.data());
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "  Parallel time : " << time_spent << "ms" << std::endl;
        }

        // Parallel swap execution ============================================================
        std::vector<std::vector<double>> A_swap(n, std::vector<double>(n));
        {
            init_array(n, A_swap.data());
            flush_cache();
            auto begin = std::chrono::high_resolution_clock::now();
            kernel_jacobi_2d_imper_swap(time_steps, n, A_swap.data());
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "  Parallel swap time : " << time_spent << "ms" << std::endl;
        }

        std::cerr << "  Compare reference with parallel execution\n";
        bool result = compare_results(n, A.data(), A_par.data());
        ck_assert(result);
        std::cerr << "  Compare reference with parallel swap execution\n";
        result = compare_results(n, A.data(), A_swap.data());
        ck_assert(result);
    }

}
END_TEST

        Suite* jacobi_suite(void)
{
    Suite* s;
    TCase* tc_core;

    s = suite_create("Jacobi-2D");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_jacobi_2d);
    suite_add_tcase(s, tc_core);

    return s;
}

int main()
{
    std::cout << "### Test Jacobi 2D Implementations ###" << std::endl;

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
