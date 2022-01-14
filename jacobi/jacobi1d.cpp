/**
 * jacobi-1d-imper.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

#include <omp.h>

#include "jacobi1d.hpp"

/* Array initialization. */
void init_1d_array(long n, double *A) {
    for (long i = 0; i < n; i++) {
        A[i] = ((double) i + 2) / n;
    }
}

void init_1d_array(long n, long offset, long size, double *A) {
    for (long i = 0; i < size; ++i) {
        A[i] = ((double) i + offset + 2) / n;
    }
}

/* Main computational kernel. The whole function will be timed, including the call and return. */
void kernel_jacobi_1d_imper(int tsteps, int n, double *A) {
    std::vector<double> B_(n);
    double *B = B_.data();
#pragma scop
    for (int t = 0; t < tsteps; t++) {
        for (int i = 1; i < n - 1; i++)
            B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);
        for (int j = 1; j < n - 1; j++)
            A[j] = B[j];
    }
#pragma endscop
}

void kernel_jacobi_1d_imper_par(int tsteps, int n, double *A) {
    std::vector<double> B_(n);
    double *B = B_.data();
    for (int t = 0; t < tsteps; t++) {
#pragma omp parallel for
        for (int i = 1; i < n - 1; i++)
            B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);

#pragma omp parallel for
        for (int j = 1; j < n - 1; j++)
            A[j] = B[j];

    }
}

void kernel_jacobi_1d_imper_swap(int tsteps, int n, double *A) {
    std::vector<double> B_(n);
    double *B = B_.data();
    B[0] = A[0];
    B[n-1] = A[n-1];
    for (int t = 0; t < tsteps; t++) {
#pragma omp parallel for
        for (int i = 1; i < n - 1; i++)
            B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);
        std::swap(A, B);
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
bool compare_results(int n, double *A, double *B) {
    constexpr int show_errors = 20;

    long errors = 0;
    long close = 0;
    for (int i = 0; i < n; i++) {
        if (A[i] != B[i]) {
            double a = std::min(A[i], B[i]), b = std::max(A[i], B[i]);
            if (!(a + std::abs(a) * allowed_relative_error > b)) { // ensure no match on NaNs
                ++errors;
                if (errors <= show_errors)
                    std::cerr << "  A[" << i << "] = " << A[i] << " ≠ B[" << i << "] = " << B[i] << "! (diff"
                              << (A[i] - B[i]) << " )\n";
            } else ++close;
        }
    }
    if (errors > show_errors) std::cerr << "    and " << (errors - show_errors) << " more\n";
    if (close) std::cerr << "  " << close << " within tolerance range (" << allowed_relative_error << ")\n";
    return errors == 0;
}
#pragma clang diagnostic pop

void read_results_file(long count, const char *filePath, double *data) {
    static_assert(sizeof(double) == 8, "8 bytes for a double expected");
    std::cout << "  Reading file…" << std::endl;
    std::ifstream fs{filePath, std::ios::binary | std::ios::in | std::ios::ate};  // ate: seek to end
    long fileSize = fs.tellg();
    if (fileSize != count * 8) {
        std::cout << "  [ERR] File size is " << fileSize << " while we expected " << count * 8 << std::endl;
        abort();
    }
    fs.seekg(std::ios::beg);  // return to start
    fs.read((char *) data, fileSize);
    std::cout << "  Successfully loaded in memory\n";
}
