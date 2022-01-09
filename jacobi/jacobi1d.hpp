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
    int ghost_cells;
    const char* output_file;

    MpiParams(int rank, int num_proc, int ghost_cells, const char* output_file) : rank(rank), num_proc(num_proc), ghost_cells(ghost_cells), output_file(output_file) {}
};

static const double allowed_relative_error = 1e-12;
void init_1d_array(long n, double *A);
void init_1d_array(long n, long offset, long size, double *A);

void read_results_file(long count, const char* filePath, double *data);
bool compare_results(int n, double *A, double *B);

void kernel_jacobi_1d_imper(int timeSteps, int n, double *A);
void kernel_jacobi_1d_imper_par(int timeSteps, int n, double *A);
void kernel_jacobi_1d_imper_barrier(int timeSteps, int n, double *A);
void jacobi_1d_imper_mpi(int timeSteps, int n, MpiParams params);
