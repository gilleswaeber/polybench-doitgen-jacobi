#include <iostream>
#include <vector>

#include <mpi.h>
#include <utils.hpp>
#include <liblsb.h>

#include "jacobi_1D.hpp"

const int RUN = 10;

struct Case {
    int n;
    int time_steps;
};

int main() {
    MPI_Init(nullptr, nullptr);

    int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) std::cout << "### MPI testing with " << num_proc << " total processes ###" << std::endl;

	LSB_Init("jacobi1d_mpi_benchmark", 0);
	LSB_Set_Rparam_string("benchmark_type", "simple");

	MPI_Barrier(MPI_COMM_WORLD);

    std::vector<Case> cases{
            {24'000, 1'000}, // initial problem size from the “Productivity, Portability, Performance: Data-Centric Python” paper
            {240'000, 10'000}, // first case is way too fast (~8ms), this one should take ~800ms/it
            {10'000'000, 1'000}  // n multiplied by ~400, should take ~4s
    };

    for (const auto &c : cases) {
        if (rank == 0) std::cout << "\n--- Jacobi 1D: n=" << c.n << " × np, time_steps=" << c.time_steps << " ---\n";
        LSB_Set_Rparam_int("base_n", c.n);
        LSB_Set_Rparam_int("time_steps", c.time_steps);
        std::vector<double> A_mpi(c.n * num_proc);

        for (int np = 1; np <= num_proc; np += (np == 1 ? 1 : 2)) {
            LSB_Set_Rparam_int("num_cores", np);
            for (int sync_steps = 1; sync_steps <= c.time_steps / np; sync_steps *= 2) {
                LSB_Set_Rparam_int("sync_steps", sync_steps);
                if (rank == 0) std::cout << "Jacobi MPI, " << RUN << " runs: n=" << c.n * np << ", time_steps=" << c.time_steps << ", np=" << np << ", sync_steps=" << sync_steps << "\n";
                for (int run = 0; run < RUN; ++run) {
                    if (rank == 0) std::cout << "\r" << run << "/" << RUN << std::flush;
                    // prepare
                    init_array(c.n * np, A_mpi.data());
                    flush_cache();
                    MPI_Barrier(MPI_COMM_WORLD);

                    LSB_Res(); // reset counters
                    // execute
                    jacobi_1d_imper_mpi(c.time_steps, c.n * np, A_mpi.data(), {rank, np, sync_steps, false});

                    LSB_Rec(run); // save run
                }
                std::cout << '\r';
            }
        }
    }

    std::cout << "Process #" << rank << " exiting\n";

	LSB_Finalize();
	MPI_Finalize();
	return 0;
}
