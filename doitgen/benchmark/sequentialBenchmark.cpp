#include <doitgen.hpp>
#include <mpi.h>
#include <utils.hpp>
#include <chrono>
#include <omp.h>
#include <liblsb.h>
#include <utils.hpp>
#include <cassert>

/**/
void init_array_seq(uint64_t nr, uint64_t nq, uint64_t np, double* a, double* c4)
{
	uint64_t i, j, k;

	for (i = 0; i < nr; i++) {
		for (j = 0; j < nq; j++) {
			for (k = 0; k < np; k++) {
				A(i, j, k) = ((double)i * j + k) / np;
			}
		}
	}

	for (i = 0; i < np; i++) {
		for (j = 0; j < np; j++) {
			C4(i, j) = ((double)i * j) / np;
		}
	}
}

//./dphpc-doitgen-sequential-benchmark 128 512 512 (for example)

int main(int argc, char **argv) {
 
    assert(argc == 6); //program name + 3 sizes

	std::string processor_model = argv[1];
	
	uint64_t run_index = strtoull(argv[2], nullptr, 10);

	uint64_t nr = strtoull(argv[3], nullptr, 10);
	uint64_t nq = strtoull(argv[4], nullptr, 10);
	uint64_t np = strtoull(argv[5], nullptr, 10);

	MPI_Init(nullptr, nullptr);
    LSB_Init((std::string("sequential_") + processor_model).c_str(), 0);

	LSB_Set_Rparam_long("NR", nr);
	LSB_Set_Rparam_long("NQ", nq);
	LSB_Set_Rparam_long("NP", np);
	LSB_Set_Rparam_long("num_processes", 1);
	LSB_Set_Rparam_long("run_index", run_index);

	LSB_Set_Rparam_string("benchmark_type", "sequential");
	LSB_Set_Rparam_string("processor_model", processor_model.c_str());

    double* a = 0;
	double* c4 = 0;
	double* sum = 0;

    a = (double*) calloc(nr * nq * np, sizeof(double));
	sum = (double*) calloc(np, sizeof(double));
	c4 = (double*) calloc(np * np, sizeof(double));

	memset(sum, 0, np);
	memset(c4, 0, np * np);

	init_array_seq(nr, nq, np, a, c4);

	uint64_t r, q, p, s;

	//kernel doigten polybench 4.0
	for (r = 0; r < nr; r++) {
		
		LSB_Res();

		for (q = 0; q < nq; q++) {
			
			for (p = 0; p < np; p++) {
				sum[p] = 0;
				for (s = 0; s < np; s++) {
					sum[p] += A(r, q, s) * C4(s, p);
				}
			}

			for (p = 0; p < np; p++) {
				A(r, q, p) = sum[p];
			}	
		}

		LSB_Rec(0);

	}

	free(a);
	free(c4);
	free(sum);

    LSB_Finalize();
	MPI_Finalize();

    return 0;
}