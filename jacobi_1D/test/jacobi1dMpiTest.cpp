#include <polybench.h>
#include <iostream>
#include <vector>

#include <mpi.h>

#define EXTRALARGE_DATASET
#include "../jacobi_1D.hpp"

bool compare_results(int n,
                     DATA_TYPE POLYBENCH_1D(A_ref, N, n),
                     DATA_TYPE POLYBENCH_1D(A, N, n)) {
    bool ok = true;
    for (int i = 0; i < n; i++) {
        if(A_ref[i] != A[i]) {
            std::cerr << "A_ref[" << i << "] = " << A_ref[i] << " ≠ A[" << i << "] = " << A[i] << "! (diff" << (A_ref[i] - A[i]) << " )\n";
            ok = false;
        }
    }
    return ok;
}

int main()
{
	MPI::Init();
    int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    POLYBENCH_1D_ARRAY_DECL(A, DATA_TYPE, N, n);

    int n = N;
    int tsteps = TSTEPS;
    init_array(n, POLYBENCH_ARRAY(A));

    if (rank == 0) {
        clock_t begin = clock();
        jacobi_1d_imper_mpi(tsteps, n, POLYBENCH_ARRAY(A));
        clock_t end = clock();
        double time_spent = (double)(end - begin);
        std::cout << "Running time : " << time_spent << std::endl;

        POLYBENCH_1D_ARRAY_DECL(A_ref, DATA_TYPE, N, n);
        POLYBENCH_1D_ARRAY_DECL(B_ref, DATA_TYPE, N, n);
        init_array(n, POLYBENCH_ARRAY(A_ref));
        kernel_jacobi_1d_imper(tsteps, n, POLYBENCH_ARRAY(A_ref), POLYBENCH_ARRAY(B_ref));
        bool ok = compare_results(n, POLYBENCH_ARRAY(A_ref), POLYBENCH_ARRAY(A));
        std::cout << "Done comparing the results: " << (ok ? "all good" : "issues were found") << "\n";

        POLYBENCH_FREE_ARRAY(A_ref);
        POLYBENCH_FREE_ARRAY(B_ref);
    } else {
        jacobi_1d_imper_mpi(tsteps, n, POLYBENCH_ARRAY(A));
    }
    //POLYBENCH_FREE_ARRAY(A);

    MPI_Finalize();

	return 0;
}
