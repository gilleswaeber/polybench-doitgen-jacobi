#include <mpi.h>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>
#include <memory>

#ifdef WITH_LSB
#include <liblsb.h>
#endif

#define ABORT_ON_ERROR(call) \
if (int status = (call)) { \
    std::cerr << "Error at " << __FILE__ << ":" << __LINE__ << ": status is " << status << std::endl; \
    abort(); \
}

void jacobi_2d_mpi(int timeSteps, int n, MpiParams params) {
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
    int gridSize = 1;
    while ((gridSize + 1) * (gridSize + 1) <= params.num_proc) ++gridSize;
    if (params.rank >= params.num_proc || params.rank >= gridSize * gridSize) {  // no work to do, still need to sync with file operations
        ABORT_ON_ERROR(MPI_File_close(&fh))
        return;
    }
    const int cellR = params.rank / gridSize;
    const int cellC = params.rank % gridSize;
    const int lastCell = gridSize - 1;

    // IMPLEMENTATION CONCEPT
    // We consider the processes as a K×K grid s.t. each process only need to sync with the one below
    // As for Jacobi-1D, we use ghost cells, i.e. we store an additional row and column on each side of the grid in
    // order to be able to compute all the values relevant to the current grid cell, then we exchange those values
    // between neighboring cells
    //
    // We distinguish:
    // - block size: the size of a block (the last block might be larger when the total size cannot be divided exactly)
    // - true size: the size of the block computed by this process
    // - padded size: the size including ghost cells

    const int blockRows = n / gridSize;
    const int trueR = params.rank * blockRows;
    const int trueRows = (cellR == lastCell ? n - ((gridSize - 1) * blockRows) : blockRows);
    const int blockCols = n / gridSize;
    const int trueC = params.rank * blockCols;
    const int trueCols = (cellC == lastCell ? n - ((gridSize - 1) * blockCols) : blockCols);

    const bool syncLeft = cellR != 0;
    const bool syncRight = cellR != lastCell;
    const bool syncTop = cellC != 0;
    const bool syncBottom = cellC != lastCell;

    const int padTop = std::min(1, trueR);
    const int padBottom = std::min(1, n - (trueR + trueRows));
    const int padLeft = std::min(1, trueC);
    const int padRight = std::min(1, n - (trueC + trueCols));
    const int paddedR = trueR - padTop;
    const int paddedRows = trueRows + padTop + padBottom;
    const int paddedC = trueC - padLeft;
    const int paddedCols = trueCols + padLeft + padRight;

    Array2dR A(paddedRows, paddedCols);
    init_2d_array(n, paddedR, paddedC, paddedRows, paddedCols, A);
#if false
#ifdef VERBOSE
    std::cout << "    MPI" << mpi.rank << '/' << mpi.num_proc << ": started for "
    << real_chunk_start << '-' << real_chunk_start + real_chunk_size
    << " (" << padded_chunk_start << '-' << padded_chunk_start + padded_chunk_size << ") "
    << A_chunk[padded_chunk_size - 1] << std::endl;
#endif

    std::vector<MPI_Request> requests((syncTop + syncBottom + syncLeft + syncRight) * 2);
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

        if (tMod == params.ghost_cells - 1) { // sync processes
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
    ABORT_ON_ERROR(MPI_File_write_at(fh, trueR * 8, trueData, trueRows, MPI_DOUBLE, MPI_STATUS_IGNORE))
    ABORT_ON_ERROR(MPI_File_close(&fh))
    // std::string split_output{mpi.output_file};
    // split_output += "r" + std::to_string(mpi.rank);
    // std::ofstream f{split_output};
    // f.write((char*) true_data, true_size * 8);
#ifdef WITH_LSB
    LSB_Rec(REC_Write);
#endif
#endif
}
