/**
 * jacobi-1d-imper.h: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */

#pragma once

struct MpiParams {
    int rank;
    int num_proc;
    int sync_steps;

    MpiParams(int rank, int numProc, int syncSteps) : rank(rank), num_proc(numProc), sync_steps(syncSteps) {}
};

static const double allowed_relative_error = 1e-12;
void init_array(int n, double *A);

bool compare_results(int n, double *A, double *B);

void kernel_jacobi_1d_imper(int timeSteps, int n, double *A);
void kernel_jacobi_1d_imper_par(int timeSteps, int n, double *A);
void kernel_jacobi_1d_imper_barrier(int timeSteps, int n, double *A);
void jacobi_1d_imper_mpi(int time_steps, int n, double *A, MpiParams params);
