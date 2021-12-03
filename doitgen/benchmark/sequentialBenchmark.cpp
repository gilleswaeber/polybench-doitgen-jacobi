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

//launch with ./exec 128 512 512 

int main(int argc, char **argv) {
 
    assert(argc == 4); //program name + 3 sizes

	uint64_t nr = strtoull(argv[1], nullptr, 10);
	uint64_t nq = strtoull(argv[2], nullptr, 10);
	uint64_t np = strtoull(argv[3], nullptr, 10);

	MPI_Init(nullptr, nullptr);
    LSB_Init("sequential", 0);

    double* a = 0;
	double* c4 = 0;
	double* sum = 0;

    a = (double*) calloc(nq * np, sizeof(double));
	sum = (double*) calloc(np, sizeof(double));
	c4 = (double*) calloc(np * np, sizeof(double));

	memset(sum, 0, np);
	memset(c4, 0, np * np);

	init_array_seq(nr, nq, np, a, c4);

	uint64_t r, q, p, s;

	//kernel doigten polybench 4.0
	for (r = 0; r < nr; r++) {
		
		//LSB_Res();

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

		//LSB_Rec(r);

	}


    LSB_Finalize();
	MPI_Finalize();

    return 0;
}