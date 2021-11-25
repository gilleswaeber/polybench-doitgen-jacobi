/**
 * jacobi-1d-imper.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <iostream>
#include <vector>
#include <cmath>

#include <mpi.h>
#include <omp.h>

#include "jacobi_1D.hpp"

/* Array initialization. */
void init_array(int n, double *A) {
    for (int i = 0; i < n; i++) {
        A[i] = ((double) i + 2) / n;
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


void kernel_jacobi_1d_imper_barrier(int tsteps, int n, double *A) {
    std::vector<double> B_(n);
    double *B = B_.data();
    for (int t = 0; t < tsteps; t++)
    {
        #pragma omp parallel
        {
            int nthreads = omp_get_num_threads();
            int tid = omp_get_thread_num();
            int low = (n - 1) * tid / nthreads;
            int high = (n - 1) * (tid + 1) / nthreads;
            // Make sure we start at 1 and not 0
            if(low == 0) {
                low = 1;
            }
            for (int i = low; i < high; i++)
                B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);

            #pragma omp barrier
            for (int j = 1; j < n - 1; j++)
                A[j] = B[j];
        }
    }
}

enum TAGS : int {
    TAG_ToPrev = 10,
    TAG_ToNext = 11,
    TAG_Collect = 12,
};

void jacobi_1d_imper_mpi(int time_steps, int n, double *A, MpiParams mpi) {
    if (mpi.rank >= mpi.num_proc) return;  // no work to do

    const int last_rank = mpi.num_proc - 1;
    const int prev_rank = mpi.rank - 1;
    const int next_rank = mpi.rank + 1;

    const int block_size = n / mpi.num_proc;
    const int real_chunk_start = mpi.rank * block_size;
    const int real_chunk_size = (mpi.rank == last_rank
                                 ? n - ((mpi.num_proc - 1) * block_size)
                                 : block_size);

    const int padded_chunk_start = real_chunk_start - (mpi.rank != 0 ? mpi.sync_steps : 0);
    const int padded_chunk_size =
            real_chunk_size + (mpi.rank != 0 ? mpi.sync_steps : 0) + (mpi.rank != last_rank ? mpi.sync_steps : 0);

    std::vector<double> A_chunk(&A[padded_chunk_start], &A[padded_chunk_start + padded_chunk_size]);
    std::vector<double> B_chunk(padded_chunk_size);
    B_chunk[0] = A_chunk[0];
    B_chunk[padded_chunk_size - 1] = A_chunk[padded_chunk_size - 1];

    if (mpi.verbose) {
        std::cout << "    MPI" << mpi.rank << '/' << mpi.num_proc << ": started for "
                  << real_chunk_start << '-' << real_chunk_start + real_chunk_size
                  << " (" << padded_chunk_start << '-' << padded_chunk_start + padded_chunk_size << ") "
                  << A_chunk[padded_chunk_size - 1] << "\n";
    }

    MPI_Request requests[4];
    MPI_Request *req_s1{&requests[0]}, *req_r1{&requests[1]}, *req_s2{&requests[2]}, *req_r2{&requests[3]};
    for (int t = 0; t < time_steps; ++t) {
        int lpad = (mpi.rank == 0 ? 1 : 1 + t % mpi.sync_steps);
        int rpad = (mpi.rank == last_rank ? 1 : 1 + t % mpi.sync_steps);
        for (int i = lpad; i < padded_chunk_size - rpad; i++) {
            B_chunk[i] = 0.33333 * (A_chunk[i - 1] + A_chunk[i] + A_chunk[i + 1]);
        }

        std::swap(A_chunk, B_chunk);

        if (t % mpi.sync_steps == mpi.sync_steps - 1) { // sync processes
            if (mpi.rank != 0) {
                MPI_Isend(A_chunk.data() + mpi.sync_steps, mpi.sync_steps, MPI_DOUBLE, prev_rank, TAG_ToPrev,
                          MPI_COMM_WORLD,
                          req_s1);
                MPI_Irecv(A_chunk.data(), mpi.sync_steps, MPI_DOUBLE, prev_rank, TAG_ToNext, MPI_COMM_WORLD, req_r1);
            }
            if (mpi.rank != last_rank) {
                MPI_Isend(A_chunk.data() + padded_chunk_size - 2 * mpi.sync_steps, mpi.sync_steps, MPI_DOUBLE,
                          next_rank,
                          TAG_ToNext, MPI_COMM_WORLD, req_s2);
                MPI_Irecv(A_chunk.data() + padded_chunk_size - mpi.sync_steps, mpi.sync_steps, MPI_DOUBLE, next_rank,
                          TAG_ToPrev,
                          MPI_COMM_WORLD, req_r2);
            }
            if (mpi.rank != 0 && mpi.rank != last_rank) MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
            else if (mpi.rank != 0) MPI_Waitall(2, &requests[0], MPI_STATUSES_IGNORE);
            else if (mpi.rank != last_rank) MPI_Waitall(2, &requests[2], MPI_STATUSES_IGNORE);
        }
    }

    // collect results in process 0
    if (mpi.rank == 0) {
        std::copy(A_chunk.data(), A_chunk.data() + real_chunk_size, A);
        std::vector<MPI_Request> collect_reqs(mpi.num_proc);
        for (int p = 1; p < mpi.num_proc; ++p) {
            int other_block_size = (p != last_rank ? block_size : n - (mpi.num_proc - 1) * block_size);
            MPI_Irecv(A + p * block_size, other_block_size, MPI_DOUBLE, p, TAG_Collect, MPI_COMM_WORLD,
                      &collect_reqs[p]);
        }
        if (mpi.num_proc > 1) MPI_Waitall(mpi.num_proc - 1, collect_reqs.data() + 1, MPI_STATUSES_IGNORE);
    } else {
        MPI_Send(A_chunk.data() + real_chunk_start - padded_chunk_start, real_chunk_size, MPI_DOUBLE, 0, TAG_Collect,
                 MPI_COMM_WORLD);
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
            if (!(a + std::abs(a) * allowed_relative_error > b)) {
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
