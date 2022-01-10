#include <iostream>
#include <vector>
#include <limits>

#include "jacobi2d.hpp"

void print_help(char *program) {
    std::cout << "Usage: " << program << " N T FILE\n"
              << "    N: array size\n"
              << "    T: time steps\n"
              << "    FILE: file to check\n"
              << std::endl;
}

int run(int n, int time_steps, const char *file_path) {
    Array2dR A_file(n, n);
    A_file.readFile(file_path);
    std::cout << "Compute using the sequential implementation…" << std::endl;
    Array2dR A_seq(n, n);
    init_2d_array(n, A_seq);
    jacobi_2d_reference(time_steps, n, A_seq);
    std::cout << "Successfully computed\n";
    std::cout << "Comparing the results…" << std::endl;
    if (A_seq.compareTo(A_file)) {
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