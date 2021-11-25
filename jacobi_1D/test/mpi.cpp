#include <iostream>
#include <vector>

#include <mpi.h>
#include <utils.hpp>
#include <chrono>

#include "jacobi_1D.hpp"

struct Case {
    int n;
    int time_steps;
    int sync_steps;
};

int main(int argc, char **argv)
{
    std::vector<Case> cases = {
            {1'000, 100, 1},
            {10'000, 200, 1},
            {10'000, 200, 8},
            {10'000, 200, 32},
            {100'000, 500, 1},
            {100'000, 500, 8},
            {100'000, 500, 32},
            {500'000, 1'000, 1},
            {500'000, 1'000, 8},
            {500'000, 1'000, 32},
    };
	MPI_Init(nullptr, nullptr);

    int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (const auto & c : cases) {
        if (rank == 0) std::cout << "Testing with n=" << c.n << ", time_steps=" << c.time_steps << ", sync_steps=" << c.sync_steps << "\n";

        std::vector<double> A_seq(c.n);
        if (rank == 0) {
            init_array(c.n, A_seq.data());
            flush_cache();
            auto begin = std::chrono::high_resolution_clock::now();
            kernel_jacobi_1d_imper(c.time_steps, c.n, A_seq.data());
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "  Sequential time : " << time_spent << "ms" << std::endl;
        }

        for (int i = 1; i <= num_proc; ++i) {
            std::vector<double> A_mpi(c.n);
            init_array(c.n, A_mpi.data());
            flush_cache();
            MPI_Barrier(MPI_COMM_WORLD);
            auto begin = std::chrono::high_resolution_clock::now();
            jacobi_1d_imper_mpi(c.time_steps, c.n, A_mpi.data(), {rank, i, c.sync_steps});
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            if (rank == 0) {
                std::cout << "  Compare with reference implementation\n";
                if (!compare_results(c.n, A_seq.data(), A_mpi.data())) {
                    std::cout << "  ! INVALID RESULTS !\n";
                };
                std::cout << "  MPI with " << i << " processes : " << time_spent << "ms" << std::endl;
            }
        }
    }

    MPI_Finalize();

	return 0;
}
