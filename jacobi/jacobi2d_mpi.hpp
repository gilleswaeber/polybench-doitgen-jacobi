// MPI Implementation for the Jacobi 2D problem
//
// IMPLEMENTATION CONCEPT
// ======================
// We consider the processes as a K×K grid s.t. each process only need to sync with the ones around
// As for Jacobi-1D, we use ghost cells, i.e. we store an additional row and column on each side of the grid in
// order to be able to compute all the values relevant to the current grid cell, then we exchange those values
// between neighboring cells
//
// For 1 single ghost cell, we only need to sync with the process up/down/left/right
// For >1 ghost cells, there are two solutions:
// DIAGONAL: we also sync with processes in diagonal
// TWO_STEPS: we sync first vertically then horizontally s.t. the diagonal cell is passed in the 2nd step
//
// We distinguish:
// - block size: the size of a block (the last block might be larger when the total size cannot be divided exactly)
// - true size: the size of the block computed by this process
// - padded size: the size including ghost cells
//
// What cells do we need to compute the Xs after ≥t steps?
//    ┃3             ┃33             ┃33⋯3┃
//   3┃23           3┃223           3┃22⋯2┃3
//  32┃123         32┃1123         32┃11⋯1┃23
// ━━━╋━━━━   …   ━━━╋━━━━━   …   ━━━╋━━⋯━╋━━━
// 321┃X123       321┃XX123       321┃XX⋯X┃123
//  32┃123        321┃XX123       321┃XX⋯X┃123
//   3┃23          32┃1123        ::::::⋱:::::
//    ┃3            3┃223         321┃XX⋯X┃123
//                   ┃33          ━━━╋━━⋯━╋━━━
//                                 32┃11⋯1┃23
//                                  3┃22⋯2┃3
//                                   ┃33⋯3┃
//
// Author: Gilles Waeber
#include <mpi.h>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>
#include <memory>
#include <algorithm>
#include "array2dr.hpp"
#include "error_handling.hpp"

#ifdef WITH_LSB
#include <liblsb.h>
#else
#include "no_lsb.hpp"
#endif

#ifndef JACOBI2D_TWO_STEPS_SYNC
#define JACOBI2D_DIAGONAL_SYNC
#endif

void jacobi_2d_mpi(int timeSteps, int n, MpiParams params) {
    enum MPI_TAG : int {
        TAG_Share = 10,
    };
    enum LSB_RECORDS : int {
        REC_Init = 0,
        REC_Compute = 1,
        REC_Sync = 2,
        REC_Write = 3,
    };
    struct Peer {
        const bool active;
        const int rank;
        const int order;
        const int bufferSize;
    private:
        std::vector<double> sendBuf;
        std::vector<double> recvBuf;
    public:
        double *send;
        const double *recv;
        Peer(bool active, int rank, int order, int bufferSize) : active(active), rank(rank), order(order),
                                                                 bufferSize(bufferSize),
                                                                 sendBuf(active ? bufferSize : 0),
                                                                 recvBuf(active ? bufferSize : 0),
                                                                 send(sendBuf.data()), recv(recvBuf.data()) {}
        bool operator<(const Peer &rhs) const { return order < rhs.order; }
        void iSync(MPI_Request *reqS, MPI_Request *reqR) {
            MPI_Isend(sendBuf.data(), bufferSize, MPI_DOUBLE, rank, TAG_Share, MPI_COMM_WORLD, reqS);
            MPI_Irecv(recvBuf.data(), bufferSize, MPI_DOUBLE, rank, TAG_Share, MPI_COMM_WORLD, reqR);
        };
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
    const int g = params.ghost_cells;

    const int blockRows = n / gridSize;
    const int trueR = cellR * blockRows;
    const int trueRows = (cellR == lastCell ? n - ((gridSize - 1) * blockRows) : blockRows);
    const int blockCols = n / gridSize;
    const int trueC = cellC * blockCols;
    const int trueCols = (cellC == lastCell ? n - ((gridSize - 1) * blockCols) : blockCols);

    // syncing across more than one cell is not supported
    ABORT_ON_ERROR(g < timeSteps && (g > blockRows || g > blockCols))

    const bool oddRow = (cellR % 2) != 0;
    const bool oddCol = (cellC % 2) != 0;

    const int padTop = std::min(g, trueR);
    const int padBottom = std::min(g, n - (trueR + trueRows));
    const int padLeft = std::min(g, trueC);
    const int padRight = std::min(g, n - (trueC + trueCols));
    const int paddedR = trueR - padTop;
    const int paddedRows = trueRows + padTop + padBottom;
    const int paddedC = trueC - padLeft;
    const int paddedCols = trueCols + padLeft + padRight;

    const int lastTrueCol = paddedCols - padRight - 1;
    const int lastTrueRow = paddedRows - padBottom - 1;

    const int ghostTriangleSize = g * (g - 1) / 2;

#ifdef JACOBI2D_DIAGONAL_SYNC
    const int vBufferSize = trueCols * g;
    const int hBufferSize = trueRows * g;
#else
    const int vBufferSize = trueCols * g;
    const int topCornerBuffer = cellR != 0 ? ghostTriangleSize : 0;
    const int bottomCornerBuffer = cellR != lastCell ? ghostTriangleSize : 0;
    const int hBufferSize = trueRows * g + topCornerBuffer + bottomCornerBuffer;
    const int topCornerStart = trueRows * g;
    const int bottomCornerStart = topCornerStart + topCornerBuffer;
#endif

    Peer top{cellR != 0, params.rank - gridSize, oddRow, vBufferSize};
    Peer bottom{cellR != lastCell, params.rank + gridSize, 1 - oddRow, vBufferSize};
    Peer left{cellC != 0, params.rank - 1, 2 + oddCol, hBufferSize};
    Peer right{cellC != lastCell, params.rank + 1, 3 - oddCol, hBufferSize};
#ifdef JACOBI2D_DIAGONAL_SYNC
    Peer topLeft{g > 1 && top.active && left.active, top.rank - 1, 4 + oddRow, ghostTriangleSize};
    Peer bottomRight{g > 1 && bottom.active && right.active, bottom.rank + 1, 5 - oddRow, ghostTriangleSize};
    Peer topRight{g > 1 && top.active && right.active, top.rank + 1, 6 + oddRow, ghostTriangleSize};
    Peer bottomLeft{g > 1 && bottom.active && left.active, bottom.rank - 1, 7 - oddRow, ghostTriangleSize};

    std::vector<Peer *> peers{}, allPeers{&top, &bottom, &left, &right, &topLeft, &bottomRight, &topRight, &bottomLeft};
    std::copy_if(allPeers.begin(), allPeers.end(), std::back_inserter(peers), [](Peer *p) { return p->active; });
    std::sort(peers.begin(), peers.end());
#else
    std::vector<Peer *> vPeers{}, hPeers{}, allVPeers{&top, &bottom}, allHPeers{&left, &right};
    std::copy_if(allVPeers.begin(), allVPeers.end(), std::back_inserter(vPeers), [](Peer *p) { return p->active; });
    std::copy_if(allHPeers.begin(), allHPeers.end(), std::back_inserter(hPeers), [](Peer *p) { return p->active; });
    std::sort(vPeers.begin(), vPeers.end());
    std::sort(hPeers.begin(), hPeers.end());
#endif

#ifdef VERBOSE
    std::cerr << "gridSize: " << gridSize << " cellR: " << cellR << " cellC: " << cellC << " lastCell: " << lastCell
    << " RANK t" << top.rank << (top.active ? "s" : "") << " b" << bottom.rank << (bottom.active ? "s" : "") << " l" << left.rank << (left.active ? "s" : "") << " r" << right.rank << (right.active ? "s" : "")
    << " PEERS " << peers.size()
    << " PAD t" << padTop << " b" << padBottom << " l" << padLeft << " r" << padRight
    << " PADDED " << paddedRows << " rows " << paddedCols << " cols from " << paddedR << "," << paddedC
    << " TRUE " << trueRows << " rows " << trueCols << " cols from " << trueR << "," << trueC << std::endl;
#endif

    Array2dR A(paddedRows, paddedCols);
    init_2d_array(n, paddedR, paddedC, paddedRows, paddedCols, A);

#ifdef VERBOSE
    std::cout << "    MPI" << params.rank << '/' << params.num_proc << ": started for "
    << trueR << "," << trueC << " - " << (trueR + trueRows) << ',' << (trueC + trueCols)
    << " (P " << paddedR << "," << paddedC << " - " << (paddedR + paddedRows) << ',' << (paddedC + paddedCols) << ") "
    << std::endl;
#endif

#ifdef JACOBI2D_DIAGONAL_SYNC
    std::vector<MPI_Request> requests(2 * peers.size());
#else
    std::vector<MPI_Request> vRequests(2 * vPeers.size());
    std::vector<MPI_Request> hRequests(2 * hPeers.size());
#endif
    LSB_Rec(REC_Init);
    std::vector<double> topRow(paddedCols);
    for (int t = 0; t < timeSteps; ++t) {
        const int tMod = t % g;
        const int leftSkip = (left.active ? 1 + tMod : 1), rightSkip = (right.active ? 1 + tMod : 1);
        const int topSkip = (top.active ? 1 + tMod : 1), bottomSkip = (bottom.active ? 1 + tMod : 1);
        memcpy(topRow.data(), A.row(topSkip - 1), paddedCols * sizeof(double));
        for (int r = topSkip; r < paddedRows - bottomSkip; ++r) {
            double leftVal = A(r, leftSkip - 1), current = A(r, leftSkip), rightVal;
            for (int c = leftSkip; c < paddedCols - rightSkip; ++c) {
                rightVal = A(r, c + 1);
                A(r, c) = .2 * (current + leftVal + rightVal + A(r + 1, c) + topRow[c]);
                topRow[c] = leftVal = current;
                current = rightVal;
            }
        }

        if (tMod == g - 1 && t != timeSteps - 1) { // sync processes
            LSB_Rec(REC_Compute);

#ifdef JACOBI2D_DIAGONAL_SYNC
            MPI_Request *req = &requests[0];
            if (top.active) A.getRectangle(padTop, padLeft, g, trueCols, top.send);
            if (bottom.active) A.getRectangle(paddedRows - 2 * g, padLeft, g, trueCols, bottom.send);
            if (left.active) A.getRectangle(padTop, padLeft, trueRows, g, left.send);
            if (right.active) A.getRectangle(padTop, paddedCols - 2 * g, trueRows, g, right.send);
            if (topLeft.active) A.getTriangle<1, 1>(padTop, padLeft, g - 1, topLeft.send);
            if (topRight.active) A.getTriangle<1, -1>(padTop, lastTrueCol, g - 1, topRight.send);
            if (bottomLeft.active) A.getTriangle<-1, 1>(lastTrueRow, padLeft, g - 1, bottomLeft.send);
            if (bottomRight.active) A.getTriangle<-1, -1>(lastTrueRow, lastTrueCol, g - 1, bottomRight.send);

            for (Peer *p : peers) {
                p->iSync(req, req + 1);
                req += 2;
            }

            if (!requests.empty()) MPI_Waitall(requests.size(), requests.data(), MPI_STATUSES_IGNORE);

            if (top.active) A.setRectangle(0, padLeft, g, trueCols, top.recv);
            if (bottom.active) A.setRectangle(paddedRows - padBottom, padLeft, g, trueCols, bottom.recv);
            if (left.active) A.setRectangle(padTop, 0, trueRows, g, left.recv);
            if (right.active) A.setRectangle(padTop, paddedCols - g, trueRows, g, right.recv);
            if (topLeft.active) A.setTriangle<-1, -1>(padTop - 1, padLeft - 1, g - 1, topLeft.recv);
            if (topRight.active) A.setTriangle<-1, 1>(padTop - 1, lastTrueCol + 1, g - 1, topRight.recv);
            if (bottomLeft.active) A.setTriangle<1, -1>(lastTrueRow + 1, padLeft - 1, g - 1, bottomLeft.recv);
            if (bottomRight.active) A.setTriangle<1, 1>(lastTrueRow + 1, lastTrueCol + 1, g - 1, bottomRight.recv);
#else
            MPI_Request *vReq = &vRequests[0];
            MPI_Request *hReq = &hRequests[0];

            // FIRST STEP: Vertical Sync
            if (top.active) A.getRectangle(padTop, padLeft, g, trueCols, top.send);
            if (bottom.active) A.getRectangle(paddedRows - 2 * g, padLeft, g, trueCols, bottom.send);
            for (Peer *p : vPeers) {
                p->iSync(vReq, vReq + 1);
                vReq += 2;
            }
            if (!vRequests.empty()) MPI_Waitall(vRequests.size(), vRequests.data(), MPI_STATUSES_IGNORE);
            if (top.active) A.setRectangle(0, padLeft, g, trueCols, top.recv);
            if (bottom.active) A.setRectangle(paddedRows - g, padLeft, g, trueCols, bottom.recv);

            // SECOND STEP: Horizontal Sync
            if (left.active) {
                A.getRectangle(padTop, padLeft, trueRows, g, left.send);
                if (top.active) A.getTriangle<-1, 1>(padTop - 1, padLeft, g - 1, left.send + topCornerStart);
                if (bottom.active) A.getTriangle<1, 1>(lastTrueRow + 1, padLeft, g - 1, left.send + bottomCornerStart);
            }
            if (right.active) {
                A.getRectangle(padTop, paddedCols - 2 * g, trueRows, g, right.send);
                if (top.active) A.getTriangle<-1, -1>(padTop - 1, lastTrueCol, g - 1, right.send + topCornerStart);
                if (bottom.active) A.getTriangle<1, -1>(lastTrueRow + 1, lastTrueCol, g - 1, right.send + bottomCornerStart);
            }
            for (Peer *p : hPeers) {
                p->iSync(hReq, hReq + 1);
                hReq += 2;
            }
            if (!hRequests.empty()) MPI_Waitall(hRequests.size(), hRequests.data(), MPI_STATUSES_IGNORE);
            if (left.active) {
                A.setRectangle(padTop, 0, trueRows, g, left.recv);
                if (top.active) A.setTriangle<-1, -1>(padTop - 1, padLeft - 1, g - 1, left.recv + topCornerStart);
                if (bottom.active) A.setTriangle<1, -1>(lastTrueRow + 1, padLeft - 1, g - 1, left.recv + bottomCornerStart);
            }
            if (right.active) {
                A.setRectangle(padTop, paddedCols - g, trueRows, g, right.recv);
                if (top.active) A.setTriangle<-1, 1>(padTop - 1, lastTrueCol + 1, g - 1, right.recv + topCornerStart);
                if (bottom.active) A.setTriangle<1, 1>(lastTrueRow + 1, lastTrueCol + 1, g - 1, right.recv + bottomCornerStart);
            }
#endif

            LSB_Rec(REC_Sync);
        }
    }
    LSB_Rec(REC_Compute);
    // write results to file
    // https://www.cscs.ch/fileadmin/user_upload/contents_publications/tutorials/fast_parallel_IO/MPI-IO_NS.pdf
    for (int r = 0; r < trueRows; ++r) {
        ABORT_ON_ERROR(MPI_File_write_at(fh,
                                         ((trueR + r) * n + trueC) * sizeof(double),
                                         A.row(padTop + r) + padLeft,
                                         trueCols,
                                         MPI_DOUBLE, MPI_STATUS_IGNORE))
    }
    ABORT_ON_ERROR(MPI_File_close(&fh))
    LSB_Rec(REC_Write);
}
