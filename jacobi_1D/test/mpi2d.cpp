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
    int sync_steps;
};

int main() {
    std::vector<Case> cases = {
            {100, 100, 1},
            {1'000, 200, 1},
            {1'000, 200, 8},
            {1'000, 200, 32},
    };
	MPI_Init(nullptr, nullptr);

    const char* temp_file = "mpi_test_temp_file~";
    int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) std::cout << "Using temp file " << temp_file << "\n";

    for (const auto & c : cases) {
        if (rank == 0) std::cout << "Testing with n=" << c.n << ", time_steps=" << c.time_steps << ", sync_steps=" << c.sync_steps << "\n";

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

        for (int i = 1; i <= num_proc; ++i) {
            flush_cache();
            if (rank == 0) unlink(temp_file);
            MPI_Barrier(MPI_COMM_WORLD);
            auto begin = std::chrono::high_resolution_clock::now();
            //MpiParams(int rank, int num_proc, int ghost_cells, const char* output_file)
            jacobi_2d_mpi(c.time_steps, c.n, {rank, i, c.sync_steps, temp_file});
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

            if (rank == 0) {
                Array2dR A_mpi(c.n, c.n);
                A_mpi.readFile(temp_file);
                std::cout << "  Compare with reference implementation\n";
                if (!A_seq.compareTo(A_mpi)) {
                    std::cout << "  ! INVALID RESULTS !\n";
                };
                std::cout << "  MPI with " << i << " processes : " << time_spent << "ms" << std::endl;
            }
        }
    }

    MPI_Finalize();

	return 0;
}
