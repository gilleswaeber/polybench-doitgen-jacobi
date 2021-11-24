/**
 * jacobi-1d-imper.h: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */

#pragma once

static const double allowed_relative_error = 1e-12;
void init_array(int n, double *A);

bool compare_results(int n, double *A, double *B);

void kernel_jacobi_1d_imper(int timeSteps, int n, double *A);
void parallel_jacobi_1d_imper(int timeSteps, int n, double *A);
void jacobi_1d_imper_mpi(int timeSteps, int n, double *A);
