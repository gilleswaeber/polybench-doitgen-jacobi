#include <iostream>
#include <vector>

#include <mpi.h>

#include "../jacobi_1D.hpp"
#include "jacobi_1D.hpp"

int main(int argc, char **argv)
{
	MPI_Init(nullptr, nullptr);

    int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int n = 100000;
    int tsteps = 1000;

    std::vector<double> A(n);
    init_array(n, A.data());

    if (rank == 0) {
        clock_t begin = clock();
        jacobi_1d_imper_mpi(tsteps, n, A.data());
        clock_t end = clock();
        double time_spent = (double)(end - begin);
        std::cout << "Running time : " << (long) time_spent << std::endl;

        std::vector<double> A_ref(n);
        init_array(n, A_ref.data());
        kernel_jacobi_1d_imper(tsteps, n, A_ref.data());
        bool ok = compare_results(n, A_ref.data(), A.data());
        std::cout << "Done comparing the results: " << (ok ? "all good" : "issues were found") << "\n";
    } else {
        jacobi_1d_imper_mpi(tsteps, n, A.data());
    }

    MPI_Finalize();

	return 0;
}
