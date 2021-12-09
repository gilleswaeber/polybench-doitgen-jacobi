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

#include <mpi.h>
#include <omp.h>
#ifdef WITH_LSB
#include <liblsb.h>
#endif

#include "jacobi_1D.hpp"

/* Array initialization. */
void init_array(long n, double *A) {
    for (long i = 0; i < n; i++) {
        A[i] = ((double) i + 2) / n;
    }
}

void init_array(long n, long offset, long size, double *A) {
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


void kernel_jacobi_1d_imper_barrier(int tsteps, int n, double *A) {
    std::vector<double> B_(n);
    double *B = B_.data();
    for (int t = 0; t < tsteps; t++) {
#pragma omp parallel
        {
            int nthreads = omp_get_num_threads();
            int tid = omp_get_thread_num();
            int low = (n - 1) * tid / nthreads;
            int high = (n - 1) * (tid + 1) / nthreads;
            // Make sure we start at 1 and not 0
            if (low == 0) {
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
};
static const int OPEN_MODE = MPI_MODE_CREATE | MPI_MODE_EXCL | MPI_MODE_WRONLY;

void jacobi_1d_imper_mpi(long time_steps, long n, MpiParams mpi) {
    MPI_File fh;
    if (mpi.rank >= mpi.num_proc) {  // no work to do, still need to sync with file operations
        if (MPI_File_open(MPI_COMM_WORLD, mpi.output_file, OPEN_MODE, MPI_INFO_NULL, &fh)) abort();
        if (MPI_File_close(&fh)) abort();
        return;
    }
    const int last_rank = mpi.num_proc - 1;
    const int prev_rank = mpi.rank - 1;
    const int next_rank = mpi.rank + 1;

    // IMPLEMENTATION CONCEPT
    // We use the ghost cells pattern, i.e. for each process, we compute the contents of a block using ghost cells on
    // the edge, i.e. cells that are part of neighboring block but necessary for the computation. The contents of those
    // ghost cells is retrieved from the neighboring blocks every couple of steps (deep halo)
    //
    // We distinguish:
    // - block size: the size of a block (the last block might be larger when the total size cannot be divided exactly)
    // - true size: the size of the block computed by this process
    // - padded size: the size including ghost cells

    const long block_size = n / mpi.num_proc;
    const long true_start = mpi.rank * block_size;
    const long true_size = (mpi.rank == last_rank
                            ? n - ((mpi.num_proc - 1) * block_size)
                            : block_size);

    const long pad_left = mpi.rank != 0 ? mpi.ghost_cells : 0;
    const long pad_right = mpi.rank != last_rank ? mpi.ghost_cells : 0;
    const long padded_start = true_start - pad_left;
    const long padded_size = true_size + pad_left + pad_right;

    std::vector<double> A_chunk(padded_size);
    init_array(n, padded_start, padded_size, A_chunk.data());
    double *true_data = A_chunk.data() + pad_left;
    std::vector<double> B_chunk(padded_size);
    B_chunk[0] = A_chunk[0];
    B_chunk[padded_size - 1] = A_chunk[padded_size - 1];

#ifdef VERBOSE
    std::cout << "    MPI" << mpi.rank << '/' << mpi.num_proc << ": started for "
              << real_chunk_start << '-' << real_chunk_start + real_chunk_size
              << " (" << padded_chunk_start << '-' << padded_chunk_start + padded_chunk_size << ") "
              << A_chunk[padded_chunk_size - 1] << std::endl;
#endif

    MPI_Request requests[4];
    MPI_Request *req_s1{&requests[0]}, *req_r1{&requests[1]}, *req_s2{&requests[2]}, *req_r2{&requests[3]};
    for (int t = 0; t < time_steps; ++t) {
#ifdef WITH_LSB
        LSB_Res();
#endif
        const long lpad = (mpi.rank == 0 ? 1 : 1 + t % mpi.ghost_cells);
        const long rpad = (mpi.rank == last_rank ? 1 : 1 + t % mpi.ghost_cells);
        for (long i = lpad; i < padded_size - rpad; i++) {
            B_chunk[i] = 0.33333 * (A_chunk[i - 1] + A_chunk[i] + A_chunk[i + 1]);
        }

        std::swap(A_chunk, B_chunk);
#ifdef WITH_LSB
        LSB_Rec(0);
#endif

        if (t % mpi.ghost_cells == mpi.ghost_cells - 1) { // sync processes
#ifdef WITH_LSB
            LSB_Res();
#endif
            if (mpi.rank != 0) {
                MPI_Isend(A_chunk.data() + mpi.ghost_cells, mpi.ghost_cells, MPI_DOUBLE, prev_rank,
                          TAG_ToPrev, MPI_COMM_WORLD, req_s1);
                MPI_Irecv(A_chunk.data(), mpi.ghost_cells, MPI_DOUBLE, prev_rank,
                          TAG_ToNext, MPI_COMM_WORLD, req_r1);
            }
            if (mpi.rank != last_rank) {
                MPI_Isend(A_chunk.data() + padded_size - 2 * mpi.ghost_cells, mpi.ghost_cells, MPI_DOUBLE, next_rank,
                          TAG_ToNext, MPI_COMM_WORLD, req_s2);
                MPI_Irecv(A_chunk.data() + padded_size - mpi.ghost_cells, mpi.ghost_cells, MPI_DOUBLE, next_rank,
                          TAG_ToPrev, MPI_COMM_WORLD, req_r2);
            }
            if (mpi.rank != 0 && mpi.rank != last_rank) MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
            else if (mpi.rank != 0) MPI_Waitall(2, &requests[0], MPI_STATUSES_IGNORE);
            else if (mpi.rank != last_rank) MPI_Waitall(2, &requests[2], MPI_STATUSES_IGNORE);
#ifdef WITH_LSB
            LSB_Rec(1);
#endif
        }
    }

    // write results to file
    // https://www.cscs.ch/fileadmin/user_upload/contents_publications/tutorials/fast_parallel_IO/MPI-IO_NS.pdf
#ifdef WITH_LSB
    LSB_Res();
#endif
    if (MPI_File_open(MPI_COMM_WORLD, mpi.output_file, OPEN_MODE, MPI_INFO_NULL, &fh)) abort();
    if (MPI_File_write_at(fh, true_start * 8, true_data, true_size, MPI_DOUBLE, MPI_STATUS_IGNORE)) abort();
    if (MPI_File_close(&fh)) abort();
#ifdef WITH_LSB
    LSB_Rec(2);
#endif
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

void read_results_file(long n, const char *file_path, double *data) {
    static_assert(sizeof(double) == 8);
    std::cout << "  Reading file…" << std::endl;
    std::ifstream fs{file_path, std::ios::binary | std::ios::in | std::ios::ate};  // ate: seek to end
    long size = fs.tellg();
    if (size != n * 8) {
        std::cout << "  [ERR] File size is " << size << " while we expected " << n * 8 << std::endl;
        abort();
    }
    fs.seekg(std::ios::beg);  // return to start
    fs.read((char *) data, size);
    std::cout << "  Successfully loaded in memory\n";
}
