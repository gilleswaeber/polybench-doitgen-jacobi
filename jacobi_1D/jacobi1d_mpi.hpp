#include <mpi.h>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>

#ifdef WITH_LSB
#include <liblsb.h>
#endif

#define ABORT_ON_ERROR(call) \
if (int status = (call)) { \
    std::cerr << "Error at " << __FILE__ << ":" << __LINE__ << ": status is " << status << std::endl; \
    abort(); \
}

void jacobi_1d_imper_mpi(int timeSteps, int n, MpiParams params) {
    enum MPI_TAG : int {
        TAG_ToPrev = 10,
        TAG_ToNext = 11,
    };
    enum LSB_RECORDS : int {
        REC_Init = 0,
        REC_Compute = 1,
        REC_Sync = 2,
        REC_Write = 3,
    };
    static const int OPEN_MODE = MPI_MODE_CREATE | MPI_MODE_EXCL | MPI_MODE_WRONLY;

    MPI_File fh;
    ABORT_ON_ERROR(MPI_File_open(MPI_COMM_WORLD, params.output_file, OPEN_MODE, MPI_INFO_NULL, &fh))
    if (params.rank >= params.num_proc) {  // no work to do, still need to sync with file operations
        ABORT_ON_ERROR(MPI_File_close(&fh))
        return;
    }
    const int lastRank = params.num_proc - 1;
    const int prevRank = params.rank - 1;
    const int nextRank = params.rank + 1;

    // IMPLEMENTATION CONCEPT
    // We use the ghost cells pattern, i.e. for each process, we compute the contents of a block using ghost cells on
    // the edge, i.e. cells that are part of neighboring block but necessary for the computation. The contents of those
    // ghost cells is retrieved from the neighboring blocks every couple of steps (deep halo)
    //
    // We distinguish:
    // - block size: the size of a block (the last block might be larger when the total size cannot be divided exactly)
    // - true size: the size of the block computed by this process
    // - padded size: the size including ghost cells

    const int blockSize = n / params.num_proc;
    const int trueStart = params.rank * blockSize;
    const int trueSize = (params.rank == lastRank
                          ? n - ((params.num_proc - 1) * blockSize)
                          : blockSize);
    const bool syncPrev = params.rank != 0;
    const bool syncNext = params.rank != lastRank;
    const bool oddRank = (params.rank % 2) == 1;

    // syncing across more than one cell is not supported
    ABORT_ON_ERROR(params.ghost_cells > blockSize && params.ghost_cells < timeSteps)

    const int padLeft = std::min(params.ghost_cells, trueStart);
    const int padRight = std::min(params.ghost_cells, n - (trueStart + trueSize));
    const int paddedStart = trueStart - padLeft;
    const int paddedSize = trueSize + padLeft + padRight;

    std::vector<double> A(paddedSize);
    init_1d_array(n, paddedStart, paddedSize, A.data());
    double *trueData = A.data() + padLeft;

#ifdef VERBOSE
    std::cout << "    MPI" << mpi.rank << '/' << mpi.num_proc << ": started for "
    << real_chunk_start << '-' << real_chunk_start + real_chunk_size
    << " (" << padded_chunk_start << '-' << padded_chunk_start + padded_chunk_size << ") "
    << A_chunk[padded_chunk_size - 1] << std::endl;
#endif

    MPI_Request requests[4];
    MPI_Request *reqSendPrev{&requests[0]}, *reqRecvPrev{&requests[1]};
    MPI_Request *reqSendNext{&requests[2]}, *reqRecvNext{&requests[3]};
#ifdef WITH_LSB
    LSB_Rec(REC_Init);
#endif
    for (int t = 0; t < timeSteps; ++t) {
        const int tMod = t % params.ghost_cells;
        const int left_skip = (params.rank == 0 ? 1 : 1 + tMod);
        const int right_skip = (params.rank == lastRank ? 1 : 1 + tMod);
        double prev = A[left_skip - 1], current = A[left_skip], next;
        for (int i = left_skip; i < paddedSize - right_skip; i++) {
            next = A[i + 1];
            A[i] = 0.33333 * (prev + current + next);
            prev = current;
            current = next;
        }

        if (tMod == params.ghost_cells - 1 && t != timeSteps - 1) { // sync processes
#ifdef WITH_LSB
            LSB_Rec(REC_Compute);
#endif
            if (syncPrev && oddRank) {
                MPI_Isend(A.data() + params.ghost_cells, params.ghost_cells, MPI_DOUBLE, prevRank,
                          TAG_ToPrev, MPI_COMM_WORLD, reqSendPrev);
                MPI_Irecv(A.data(), params.ghost_cells, MPI_DOUBLE, prevRank,
                          TAG_ToNext, MPI_COMM_WORLD, reqRecvPrev);
            }
            if (syncNext) {
                MPI_Isend(A.data() + paddedSize - 2 * params.ghost_cells, params.ghost_cells, MPI_DOUBLE, nextRank,
                          TAG_ToNext, MPI_COMM_WORLD, reqSendNext);
                MPI_Irecv(A.data() + paddedSize - params.ghost_cells, params.ghost_cells, MPI_DOUBLE, nextRank,
                          TAG_ToPrev, MPI_COMM_WORLD, reqRecvNext);
            }
            if (syncPrev && !oddRank) {
                MPI_Isend(A.data() + params.ghost_cells, params.ghost_cells, MPI_DOUBLE, prevRank,
                          TAG_ToPrev, MPI_COMM_WORLD, reqSendPrev);
                MPI_Irecv(A.data(), params.ghost_cells, MPI_DOUBLE, prevRank,
                          TAG_ToNext, MPI_COMM_WORLD, reqRecvPrev);
            }
            if (syncPrev && syncNext) MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
            else if (syncPrev) MPI_Waitall(2, &requests[0], MPI_STATUSES_IGNORE);
            else if (syncNext) MPI_Waitall(2, &requests[2], MPI_STATUSES_IGNORE);
#ifdef WITH_LSB
            LSB_Rec(REC_Sync);
#endif
        }
    }
#ifdef WITH_LSB
    LSB_Rec(REC_Compute);
#endif
    // write results to file
    // https://www.cscs.ch/fileadmin/user_upload/contents_publications/tutorials/fast_parallel_IO/MPI-IO_NS.pdf
    ABORT_ON_ERROR(MPI_File_write_at(fh, trueStart * 8, trueData, trueSize, MPI_DOUBLE, MPI_STATUS_IGNORE))
    ABORT_ON_ERROR(MPI_File_close(&fh))
    // std::string split_output{mpi.output_file};
    // split_output += "r" + std::to_string(mpi.rank);
    // std::ofstream f{split_output};
    // f.write((char*) true_data, true_size * 8);
#ifdef WITH_LSB
    LSB_Rec(REC_Write);
#endif
}