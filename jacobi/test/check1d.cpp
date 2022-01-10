#include <iostream>
#include <vector>
#include <limits>

#include "jacobi1d.hpp"

void print_help(char *program) {
    std::cout << "Usage: " << program << " N T FILE\n"
              << "    N: array size\n"
              << "    T: time steps\n"
              << "    FILE: file to check\n"
              << std::endl;
}

int run(int n, int time_steps, const char *file_path) {
    std::vector<double> A_file(n);
    read_results_file(n, file_path, A_file.data());
    std::cout << "Compute using the sequential implementation…" << std::endl;
    std::vector<double> A_seq(n);
    init_1d_array(n, A_seq.data());
    kernel_jacobi_1d_imper(time_steps, n, A_seq.data());
    std::cout << "Successfully computed\n";
    std::cout << "Comparing the results…" << std::endl;
    if (compare_results(n, A_seq.data(), A_file.data())) {
        std::cout << "The results are correct\n";
        return 0;
    } else {
        std::cout << "INVALID RESULTS !\n";
        return 3;
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        print_help(argv[0]);
        return 1;
    }
    long n = strtol(argv[1], nullptr, 10);
    if (errno || n <= 3 || n > std::numeric_limits<int>::max()) {
        print_help(argv[0]);
        return 1;
    }
    long time_steps = strtol(argv[2], nullptr, 10);
    if (errno || time_steps < 0 || time_steps > std::numeric_limits<int>::max()) {
        print_help(argv[0]);
        return 1;
    }
    return run(n, time_steps, argv[3]);
}