#include <vector>
#include <omp.h>
#include <iostream>

#include <mpi.h>
#include <liblsb.h>

#include "jacobi_1D.hpp"

#define RUNS 10
#define THREADS_SIZES 7
const static int threads[] = { 1, 2, 4, 8, 16, 32, 48 };

int main() {
    int n = 500000;
    int time_steps = 1000;
    std::vector<double> A(n);
    std::vector<double> A_par(n);
    std::vector<double> A_barrier(n);

    MPI_Init(nullptr, nullptr);

    /* Initialize lsblib */
    LSB_Init("jacobi-openMP", 0);

    LSB_Set_Rparam_int("N", n);
    LSB_Set_Rparam_int("TSTEPS", time_steps);

    // Sequential benchmark
    LSB_Set_Rparam_string("benchmark", "jacobi-sequential");
    LSB_Set_Rparam_int("threads", threads[0]);

    for (uint64_t i = 0; i < RUNS; ++i) {
        init_1d_array(n, A.data());

        LSB_Res();
        kernel_jacobi_1d_imper(time_steps, n, A.data());
        LSB_Rec(i);
    }

    // Parallel benchmark
    LSB_Set_Rparam_string("benchmark", "jacobi-parallel");
    for (uint64_t i = 0; i < THREADS_SIZES; ++i) {
        omp_set_num_threads(threads[i]);
        LSB_Set_Rparam_int("threads", threads[i]);

        for (uint64_t j = 0; j < RUNS; ++j) {
            init_1d_array(n, A_par.data());

            LSB_Res();
            kernel_jacobi_1d_imper_par(time_steps, n, A_par.data());
            LSB_Rec(j);
        }
    }

    // Parallel barrier benchmark
    LSB_Set_Rparam_string("benchmark", "jacobi-parallel");
    for (uint64_t i = 0; i < THREADS_SIZES; ++i) {
        omp_set_num_threads(threads[i]);
        LSB_Set_Rparam_int("threads", threads[i]);

        for (uint64_t j = 0; j < RUNS; ++j) {
            init_1d_array(n, A_barrier.data());

            LSB_Res();
            kernel_jacobi_1d_imper_barrier(time_steps, n, A_barrier.data());
            LSB_Rec(j);
        }
    }

    /* Finalize lsblib */
    LSB_Finalize();

    MPI_Finalize();

    return 0;
}
