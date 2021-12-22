#include <iostream>

#include <mpi.h>
#ifdef WITH_LSB
#include <liblsb.h>
#endif

#include "jacobi_1D.hpp"
#include "jacobi1d_mpi.hpp"

void print_help(char *program) {
    std::cout << "Usage: " << program << " N T S FILE\n"
              << "    N: array size, multiplied by the number of cores\n"
              << "    T: time steps\n"
              << "    S: number of ghost cells to use, 1 <= S < N/2\n"
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

    jacobi_1d_imper_mpi(time_steps, n * num_proc, {rank, num_proc, ghost_cells, output_file}); // execute

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
    if (errno || ghost_cells < 1 || ghost_cells >= n / 2) {
        print_help(argv[0]);
        return 1;
    }
    const char *output_file = argv[4];
    run(n, time_steps, ghost_cells, output_file);
    return 0;
}
