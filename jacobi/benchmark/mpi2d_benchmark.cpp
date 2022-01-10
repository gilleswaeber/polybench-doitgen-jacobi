// Benchmark the MPI Jacobi 2D implementation
// Set the JACOBI2D_TWO_STEPS_SYNC compile flag to use the
// Author: Gilles Waeber
#include <iostream>

#include <mpi.h>
#ifdef WITH_LSB
#include <liblsb.h>
#endif

#include "jacobi2d.hpp"
#include "jacobi2d_mpi.hpp"

void print_help(char *program) {
    std::cout << "Usage: " << program << " N T S FILE\n"
              << "    N: array size, multiplied by the sqrt of the number of cores\n"
              << "    T: time steps\n"
              << "    S: number of ghost cells to use, 1 <= S\n"
              << "    FILE: output file for computed data\n"
              << std::endl;
}

void run(long n, long time_steps, int ghost_cells, const char* output_file) {
    MPI_Init(nullptr, nullptr);

    int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#ifdef VERBOSE
    if (rank == 0) std::cout << "### MPI testing with " << num_proc << " total processes ###" << std::endl;
#endif
#ifdef WITH_LSB
    LSB_Init("jacobi1d_mpi_benchmark", 0);
    MPI_Barrier(MPI_COMM_WORLD);
    LSB_Res();
#endif

    int scale = 1;
    while ((scale + 1) * (scale + 1) <= num_proc) ++scale;
    jacobi_2d_mpi(time_steps, scale * n, {rank, num_proc, ghost_cells, output_file}); // execute

#ifdef WITH_LSB
    LSB_Finalize();
#endif
    MPI_Finalize();
}

int main(int argc, char **argv) {
    std::ios_base::sync_with_stdio(false);
    if (argc != 5) {
        print_help(argv[0]);
        return 1;
    }
    long n = strtol(argv[1], nullptr, 10);
    if (errno || n <= 3) {
        print_help(argv[0]);
        return 1;
    }
    long time_steps = strtol(argv[2], nullptr, 10);
    if (errno) {
        print_help(argv[0]);
        return 1;
    }
    long ghost_cells = strtol(argv[3], nullptr, 10);
    if (errno || n < 1) {
        print_help(argv[0]);
        return 1;
    }
    const char *output_file = argv[4];
    run(n, time_steps, ghost_cells, output_file);
    return 0;
}
