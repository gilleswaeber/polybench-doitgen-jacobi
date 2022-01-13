// Author: Gilles Waeber
#include <iostream>
#include <vector>

#include <mpi.h>
#include <utils.hpp>
#include <chrono>
#include <unistd.h>

#include "jacobi2d.hpp"
#include "jacobi2d_mpi.hpp"

struct Case {
    int n;
    int time_steps;
    std::vector<int> ghost_cells;
};

int main() {
    std::vector<Case> cases = {
            {10, 10, {1}},
            {100, 100, {100}},
            {200, 200, {1, 4, 10}},
            {1'000, 300, {1, 8, 20}},
    };
	MPI_Init(nullptr, nullptr);

    const char* temp_file = "mpi_test_temp_file~";
    int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) std::cout << "Using temp file " << temp_file << "\n";

    for (const auto & c : cases) {
        if (rank == 0) std::cout << "Testing with n=" << c.n << ", time_steps=" << c.time_steps << "\n";

        Array2dR A_seq(c.n, c.n);
        if (rank == 0) {
            init_2d_array(c.n, A_seq);
            flush_cache();
            auto begin = std::chrono::high_resolution_clock::now();
            jacobi_2d_reference(c.time_steps, c.n, A_seq);
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "  Sequential time : " << time_spent << "ms" << std::endl;
        }

        for (int g : c.ghost_cells)
        for (int i = 1; i*i <= num_proc; ++i) {
            flush_cache();
            if (rank == 0) unlink(temp_file);
            MPI_Barrier(MPI_COMM_WORLD);
#ifdef JACOBI2D_VSTACK
            if (g < c.time_steps && g > c.n / (i*i)) {
                if (rank == 0) std::cout << "  Not possible with " << i*i << " processes and " << g << " ghost cells\n";
                continue;
            }
#endif
            auto begin = std::chrono::high_resolution_clock::now();
            //MpiParams(int rank, int num_proc, int ghost_cells, const char* output_file)
            jacobi_2d_mpi(c.time_steps, c.n, {rank, i*i, g, temp_file});
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

            if (rank == 0) {
                Array2dR A_mpi(c.n, c.n);
                A_mpi.readFile(temp_file);
                std::cout << "  MPI with " << i*i << " processes, " << g << " ghost cells : " << time_spent << "ms" << std::endl;
                std::cout << "  Compare with reference implementation\n";
                if (!A_seq.compareTo(A_mpi)) {
                    std::cout << "  ! INVALID RESULTS !\n";
                    abort();
                };
            }
        }
    }

    MPI_Finalize();

	return 0;
}
