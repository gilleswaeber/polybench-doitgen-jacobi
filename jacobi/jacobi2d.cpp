#include "jacobi2d.hpp"
// Author: Gilles Waeber

void jacobi_2d_reference(int timeSteps, int n, Array2dR &A) {
    Array2dR B{n, n};
    for (int t = 0; t < timeSteps; t++) {
        for (int i = 1; i < n - 1; i++)
            for (int j = 1; j < n - 1; j++)
                B(i, j) = 0.2 * (A(i, j) + A(i, j - 1) + A(i, 1 + j) + A(1 + i, j) + A(i - 1, j));
        for (int i = 1; i < n - 1; i++)
            for (int j = 1; j < n - 1; j++)
                A(i, j) = B(i, j);
    }
}

void init_2d_array(int n, int firstRow, int firstCol, int rows, int cols, Array2dR &A) {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // A(r, c) = ((double) r * (c + 2) + 2) / n;
            A(r, c) = ((double) (firstRow + r) * ((firstCol + c) + 2) + 2) / n;
        }
    }
}

void init_2d_array(int n, Array2dR &A) {
    init_2d_array(n, 0, 0, n, n, A);
}