#include <vector>

#include "jacobi2d_omp.hpp"

/* Array initialization. */
void init_array(int n, std::vector<double> *A) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            A[i][j] = ((double)i * (j + 2) + 2) / n;
        }
    }
}

/* Main computational kernel. */
void kernel_jacobi_2d_imper(int tsteps, int n, std::vector<double> *A) {
    std::vector<std::vector<double>> B_(n, std::vector<double>(n));
    std::vector<double> *B = B_.data();

#pragma scop
    for (int t = 0; t < tsteps; t++) {
        for (int i = 1; i < n - 1; i++) {
            for (int j = 1; j < n - 1; j++) {
                B[i][j] = 0.2 * (A[i][j] + A[i][j - 1] + A[i][1 + j] + A[1 + i][j] + A[i - 1][j]);
            }
        }
        for (int i = 1; i < n - 1; i++) {
            for (int j = 1; j < n - 1; j++) {
                A[i][j] = B[i][j];
            }
        }
    }
#pragma endscop
}


void kernel_jacobi_2d_imper_par(int tsteps, int n, std::vector<double> *A) {
    std::vector<std::vector<double>> B_(n, std::vector<double>(n));
    std::vector<double> *B = B_.data();

    for (int t = 0; t < tsteps; t++) {
#pragma omp parallel for
        for (int i = 1; i < n - 1; i++) {
#pragma omp parallel for
            for (int j = 1; j < n - 1; j++) {
                B[i][j] = 0.2 * (A[i][j] + A[i][j - 1] + A[i][1 + j] + A[1 + i][j] + A[i - 1][j]);
            }
        }
#pragma omp parallel for
        for (int i = 1; i < n - 1; i++) {
#pragma omp parallel for
            for (int j = 1; j < n - 1; j++) {
                A[i][j] = B[i][j];
            }
        }
    }
}

void kernel_jacobi_2d_imper_swap(int tsteps, int n, std::vector<double> *A) {
    std::vector<double> *B = A;

    for (int t = 0; t < tsteps; t++) {
#pragma omp parallel for
        for (int i = 1; i < n - 1; i++) {
#pragma omp parallel for
            for (int j = 1; j < n - 1; j++) {
                B[i][j] = 0.2 * (A[i][j] + A[i][j - 1] + A[i][1 + j] + A[1 + i][j] + A[i - 1][j]);
            }
        }
        std::swap(A, B);
    }
}