#pragma once

#include <vector>
#include <ostream>
#include <fstream>
#include "jacobi_1D.hpp"

struct Array2dR {
    // Row-first 2D matrix
    const int rows, cols;
    Array2dR(int rows, int cols) : rows{rows}, cols{cols}, data(rows * cols) {}
    inline double& operator()(int row, int col) { return at(row, col); }
    inline const double& operator()(int row, int col) const { return at(row, col); }

    void readFile(const char* filePath) const {
        std::ifstream fs{filePath, std::ios::binary | std::ios::in | std::ios::ate};  // ate: seek to end
        const long fileSize = fs.tellg();
        if (fileSize != rows * cols * sizeof(double)) {
            std::cout << "  [ERR] File size is " << fileSize << " while we expected " << rows * cols * sizeof(double) << std::endl;
            abort();
        }
        fs.seekg(std::ios::beg);  // return to start
        fs.read((char *) data.data(), fileSize);
    }

    bool compareTo(const Array2dR& other) const {
        constexpr int showErrors = 20;

        if (rows != other.rows || cols != other.cols) {
            std::cerr << "  Size mismatch A is " << rows << "x" << cols << ", B is " << other.rows << "x" << other.cols << '\n';
            return false;
        }

        long errors = 0;
        long close = 0;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (at(r, c) != other.at(r, c)) {
                    double a = std::min(at(r, c), other.at(r, c)), b = std::max(at(r, c), other.at(r, c));
                    if (!(a + std::abs(a) * allowed_relative_error > b)) { // ensure no match on NaNs
                        ++errors;
                        if (errors <= showErrors)
                            std::cerr << "  A[" << r << ',' << c << "] = " << at(r, c) << " ≠ B[" << r << ',' << c << "] = " << other.at(r, c) << "! (diff"
                            << (at(r, c) - other.at(r, c)) << " )\n";
                    } else ++close;
                }
            }
        }
        if (errors > showErrors) std::cerr << "    and " << (errors - showErrors) << " more\n";
        if (close) std::cerr << "  " << close << " within tolerance range (" << allowed_relative_error << ")\n";
        return errors == 0;
    }
private:
    inline const double& at(int row, int col) const { return data[row * rows + col]; }
    inline double& at(int row, int col) { return data[row * rows + col]; }
    std::vector<double> data;
};

void init_2d_array(int n, Array2dR &A) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A(i, j) = ((double) i * (j + 2) + 2) / n;
        }
    }
}
void init_2d_array(int n, int firstRow, int firstCol, int rows, int cols, Array2dR &A) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            A(i, j) = ((double) (firstRow + i) * ((firstCol + j) + 2) + 2) / n;
        }
    }
}
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
void jacobi_2d_mpi(int timeSteps, int n, MpiParams params);