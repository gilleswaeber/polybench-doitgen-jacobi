#pragma once
// Row-first 2D matrix
// Author: Gilles Waeber

#include <iostream>
#include <cstring>

struct Array2dR {
    // Row-first 2D matrix
    const int rows, cols;
    Array2dR(int rows, int cols) : rows{rows}, cols{cols}, data(rows * cols) {}
    inline double& operator()(int row, int col) { return at(row, col); }
    inline const double& operator()(int row, int col) const { return at(row, col); }
    inline double* row(int r) { return data.data() + r * cols; }
    inline const double* row(int r) const { return data.data() + r * cols; }

    void readFile(const char* filePath) const {
        std::ifstream fs{filePath, std::ios::binary | std::ios::in | std::ios::ate};  // ate: seek to end
        const long fileSize = fs.tellg();
        if (fileSize != (rows * cols * sizeof(double))) {
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
                    if (std::isnan(a) || std::isnan(b) || a + std::abs(a) * allowed_relative_error <= b) {
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

    void getRectangle(int r0, int c0, int rowCount, int colCount, double *dest) const {
        assert(r0 >= 0 && r0 + rowCount <= rows);
        assert(c0 >= 0 && c0 + colCount <= cols);
        for (int r = 0; r < rowCount; ++r) {
            memcpy(dest + r * colCount, row(r0 + r) + c0, colCount * sizeof(double));
        }
    }
    void setRectangle(int r0, int c0, int rowCount, int colCount, const double *src) {
        assert(r0 >= 0 && r0 + rowCount <= rows);
        assert(c0 >= 0 && c0 + colCount <= cols);
        for (int r = 0; r < rowCount; ++r) {
            memcpy(row(r0 + r) + c0, src + r * colCount, colCount * sizeof(double));
        }
    }
    template<int RowDir, int ColDir>
    void getTriangle(int r0, int c0, int n, double *dest) const {
        // Triangle with a 90° angle at the (r0,c0) corner and adjacent sides have length n
        static_assert(RowDir == 1 || RowDir == -1, "RowDir must be 1 or -1");
        static_assert(ColDir == 1 || ColDir == -1, "ColDir must be 1 or -1");
        for (int r = 0; r < n; ++r) {
            memcpy(dest, row(r0 + r * RowDir) + (ColDir == 1 ? c0 : c0 - n + r + 1), (n - r) * sizeof(double));
            dest += n - r;
        }
    }
    template<int RowDir, int ColDir>
    void setTriangle(int r0, int c0, int n, const double *src) {
        static_assert(RowDir == 1 || RowDir == -1, "RowDir must be 1 or -1");
        static_assert(ColDir == 1 || ColDir == -1, "ColDir must be 1 or -1");
        for (int r = 0; r < n; ++r) {
            memcpy(row(r0 + r * RowDir) + (ColDir == 1 ? c0 : c0 - n + r + 1), src, (n - r) * sizeof(double));
            src += n - r;
        }
    }
private:
    inline const double& at(int row, int col) const { return data[row * cols + col]; }
    inline double& at(int row, int col) { return data[row * cols + col]; }
    std::vector<double> data;
};
