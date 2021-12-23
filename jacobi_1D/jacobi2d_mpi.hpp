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
    if (params.rank >= params.num_proc ||
        params.rank >= gridSize * gridSize) {  // no work to do, still need to sync with file operations
        ABORT_ON_ERROR(MPI_File_close(&fh))
        return;
    }
    const int cellR = params.rank / gridSize;
    const int cellC = params.rank % gridSize;
    const int lastCell = gridSize - 1;

    const int rankTop = params.rank - gridSize;
    const int rankBottom = params.rank + gridSize;
    const int rankLeft = params.rank - 1;
    const int rankRight = params.rank + 1;

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
    const int trueR = cellR * blockRows;
    const int trueRows = (cellR == lastCell ? n - ((gridSize - 1) * blockRows) : blockRows);
    const int blockCols = n / gridSize;
    const int trueC = cellC * blockCols;
    const int trueCols = (cellC == lastCell ? n - ((gridSize - 1) * blockCols) : blockCols);

    const bool syncTop = cellR != 0;
    const bool syncBottom = cellR != lastCell;
    const bool syncLeft = cellC != 0;
    const bool syncRight = cellC != lastCell;

    const int padTop = std::min(1, trueR);
    const int padBottom = std::min(1, n - (trueR + trueRows));
    const int padLeft = std::min(1, trueC);
    const int padRight = std::min(1, n - (trueC + trueCols));
    const int paddedR = trueR - padTop;
    const int paddedRows = trueRows + padTop + padBottom;
    const int paddedC = trueC - padLeft;
    const int paddedCols = trueCols + padLeft + padRight;

#ifdef VERBOSE
    std::cerr << "gridSize: " << gridSize << " cellR: " << cellR << " cellC: " << cellC << " lastCell: " << lastCell
    << " RANK t" << rankTop << (syncTop ? "s" : "") << " b" << rankBottom << (syncBottom ? "s" : "") << " l" << rankLeft << (syncLeft ? "s" : "") << " r" << rankRight << (syncRight ? "s" : "")
    << " PAD t" << padTop << " b" << padBottom << " l" << padLeft << " r" << padRight
    << " PADDED " << paddedRows << " rows " << paddedCols << " cols from " << paddedR << "," << paddedC
    << " TRUE " << trueRows << " rows " << trueCols << " cols from " << trueR << "," << trueC << std::endl;
#endif

    Array2dR A(paddedRows, paddedCols);
    init_2d_array(n, paddedR, paddedC, paddedRows, paddedCols, A);

    std::vector<double> sendLeft(trueRows), recvLeft(trueRows);
    std::vector<double> sendRight(trueRows), recvRight(trueRows);

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
    std::vector<double> top(paddedCols);
    for (int t = 0; t < timeSteps; ++t) {
        const int leftSkip = 1, rightSkip = 1;
        const int topSkip = 1, bottomSkip = 1;
        memcpy(top.data(), A.row(0), paddedCols * sizeof(double));
        for (int r = topSkip; r < paddedRows - bottomSkip; ++r) {
            double left = A(r, 0), current = A(r, 1), right;
            for (int c = leftSkip; c < paddedCols - rightSkip; ++c) {
                right = A(r, c + 1);
                A(r, c) = .2 * (current + left + right + A(r + 1, c) + top[c]);
                top[c] = left = current;
                current = right;
            }
        }

        { // sync processes
#ifdef WITH_LSB
            LSB_Rec(REC_Compute);
#endif
            MPI_Request *req = &requests[0];
            if (syncTop) {
                MPI_Isend(A.row(1) + padLeft, trueCols, MPI_DOUBLE, rankTop, TAG_ToPrev, MPI_COMM_WORLD, req++);
                MPI_Irecv(A.row(0) + padLeft, trueCols, MPI_DOUBLE, rankTop, TAG_ToNext, MPI_COMM_WORLD, req++);
            }
            if (syncBottom) {
                MPI_Isend(A.row(padTop + trueRows - 1) + padLeft, trueCols, MPI_DOUBLE, rankBottom, TAG_ToNext, MPI_COMM_WORLD, req++);
                MPI_Irecv(A.row(padTop + trueRows) + padLeft, trueCols, MPI_DOUBLE, rankBottom, TAG_ToPrev, MPI_COMM_WORLD, req++);
            }
            if (syncLeft) {
                for (int r = 0; r < trueRows; ++r) sendLeft[r] = A(padTop + r, 1);
                MPI_Isend(sendLeft.data(), trueRows, MPI_DOUBLE, rankLeft, TAG_ToPrev, MPI_COMM_WORLD, req++);
                MPI_Irecv(recvLeft.data(), trueRows, MPI_DOUBLE, rankLeft, TAG_ToNext, MPI_COMM_WORLD, req++);
            }
            if (syncRight) {
                for (int r = 0; r < trueRows; ++r) sendRight[r] = A(padTop + r, paddedCols - 2);
                MPI_Isend(sendRight.data(), trueRows, MPI_DOUBLE, rankRight, TAG_ToNext, MPI_COMM_WORLD, req++);
                MPI_Irecv(recvRight.data(), trueRows, MPI_DOUBLE, rankRight, TAG_ToPrev, MPI_COMM_WORLD, req++);
            }
            if (requests.size()) MPI_Waitall(requests.size(), requests.data(), MPI_STATUSES_IGNORE);
            if (syncLeft) {
                for (int r = 0; r < trueRows; ++r) A(padTop + r, 0) = recvLeft[r];
            }
            if (syncRight) {
                for (int r = 0; r < trueRows; ++r) A(padTop + r, paddedCols - 1) = recvRight[r];
            }
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
    // for (int r = 0; r < paddedRows; ++r) {
    //     for (int c = 0; c < paddedCols; ++c) {
    //         A(r, c) = 100000 * (paddedR + r) + 100 * (paddedC + c) + 1 * (params.rank + 1);
    //     }
    // }

    for (int r = 0; r < trueRows; ++r) {
        ABORT_ON_ERROR(MPI_File_write_at(fh, ((trueR + r) * n + trueC) * 8, A.row(padTop + r) + padLeft, trueCols, MPI_DOUBLE, MPI_STATUS_IGNORE))
    }
    ABORT_ON_ERROR(MPI_File_close(&fh))
    // std::string split_output{mpi.output_file};
    // split_output += "r" + std::to_string(mpi.rank);
    // std::ofstream f{split_output};
    // f.write((char*) true_data, true_size * 8);
#ifdef WITH_LSB
    LSB_Rec(REC_Write);
#endif
}
