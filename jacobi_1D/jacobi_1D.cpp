/**
 * jacobi-1d-imper.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
/* Include polybench common header. */
#include <iostream>
#include <vector>

#include <mpi.h>

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

void parallel_jacobi_1d_imper(int tsteps, int n, double *A) {
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

//    for (int t = 0; t < tsteps; t++)
//    {
//        #pragma omp parallel
//        {
//            int nthreads = omp_get_num_threads();
//            int tid = omp_get_thread_num();
//            int low = (n - 1) * tid / nthreads;
//            int high = (n - 1) * (tid + 1) / nthreads;
//            // Make sure we start at 1 and not 0
//            if(low == 0) {
//                low = 1;
//            }
//            for (int i = low; i < high; i++)
//                B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);
//
//            #pragma omp barrier
//            for (int j = 1; j < n - 1; j++)
//                A[j] = B[j];
//        }
//    }
}

enum TAGS : int {
    TAG_ToPrev = 10,
    TAG_ToNext = 11,
    TAG_Collect = 12,
};

void jacobi_1d_imper_mpi(int tsteps, int n, double *A) {
    int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int last_rank = num_proc - 1;

    int block_size = n / num_proc;
    int sync_every = 1;
    int real_chunk_start = rank * block_size;
    int real_chunk_size;

    if (rank == last_rank) {
        real_chunk_size = n - ((num_proc - 1) * block_size);
    } else {
        real_chunk_size = block_size;
    }

    int padded_chunk_start = real_chunk_start - (rank != 0 ? sync_every : 0);
    int padded_chunk_size = real_chunk_size + (rank != 0 ? sync_every : 0) + (rank != last_rank ? sync_every : 0);

    std::vector<double> A_chunk(&A[padded_chunk_start], &A[padded_chunk_start + padded_chunk_size]);
    std::vector<double> B_chunk(padded_chunk_size);
    B_chunk[0] = A_chunk[0];
    B_chunk[padded_chunk_size - 1] = A_chunk[padded_chunk_size - 1];

    std::cout << "    MPI" << rank << '/' << num_proc << ": started for "
              << real_chunk_start << '-' << real_chunk_start + real_chunk_size
              << " (" << padded_chunk_start << '-' << padded_chunk_start + padded_chunk_size << ") "
              << A_chunk[padded_chunk_size - 1] << "\n";

    MPI_Request requests[4];
    MPI_Request *req_s1{&requests[0]}, *req_r1{&requests[1]}, *req_s2{&requests[2]}, *req_r2{&requests[3]};
    for (int t = 0; t < tsteps; ++t) {
        for (int i = 1; i < padded_chunk_size - 1; i++)
            B_chunk[i] = 0.33333 * (A_chunk[i - 1] + A_chunk[i] + A_chunk[i + 1]);
        std::swap(A_chunk, B_chunk);

        if (t % sync_every == 0) { // sync processes
            if (rank != 0) {
                MPI_Isend(A_chunk.data() + sync_every, sync_every, MPI_DOUBLE, rank - 1, TAG_ToPrev, MPI_COMM_WORLD,
                          req_s1);
                MPI_Irecv(A_chunk.data(), sync_every, MPI_DOUBLE, rank - 1, TAG_ToNext, MPI_COMM_WORLD, req_r1);
            }
            if (rank != last_rank) {
                MPI_Isend(A_chunk.data() + padded_chunk_size - 2 * sync_every, sync_every, MPI_DOUBLE, rank + 1,
                          TAG_ToNext, MPI_COMM_WORLD, req_s2);
                MPI_Irecv(A_chunk.data() + padded_chunk_size - sync_every, sync_every, MPI_DOUBLE, rank + 1, TAG_ToPrev,
                          MPI_COMM_WORLD, req_r2);
            }
            if (rank != 0 && rank != last_rank) MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
            else if (rank != 0) MPI_Waitall(2, &requests[0], MPI_STATUSES_IGNORE);
            else if (rank != last_rank) MPI_Waitall(2, &requests[2], MPI_STATUSES_IGNORE);
        }
    }

    // collect results in process 0
    if (rank == 0) {
        std::copy(A_chunk.data(), A_chunk.data() + real_chunk_size, A);
        std::vector<MPI_Request> collect_reqs(num_proc);
        for (int p = 1; p < num_proc; ++p) {
            int other_block_size = (p != last_rank ? block_size : n - (num_proc - 1) * block_size);
            MPI_Irecv(A + p * block_size, other_block_size, MPI_DOUBLE, p, TAG_Collect, MPI_COMM_WORLD,
                      &collect_reqs[p]);
        }
        if (num_proc > 1) MPI_Waitall(num_proc - 1, collect_reqs.data() + 1, MPI_STATUSES_IGNORE);
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
        if(A[i] != B[i]) {
            double a = std::min(A[i], B[i]), b = std::max(A[i], B[i]);
            if (!(a + std::abs(a) * allowed_relative_error > b)) {
                ++errors;
                if (errors <= show_errors) std::cerr << "  A[" << i << "] = " << A[i] << " ≠ B[" << i << "] = " << B[i] << "! (diff" << (A[i] - B[i]) << " )\n";
            } else ++close;
        }
    }
    if (errors > show_errors) std::cerr << "    and " << (errors - show_errors) << " more\n";
    if (close) std::cerr << "  " << close << " within tolerance range (" << allowed_relative_error << ")\n";
    return errors == 0;
}
#pragma clang diagnostic pop