#include <doitgen.hpp>
#include <utils.hpp>
#include <chrono>
#include <omp.h>
#include <liblsb.h>
#include <utils.hpp>

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

int main(int argc, char **argv) {
 
    //assert(argc == 4); //program name + 3 sizes

	char* output_path = argv[1];

	uint64_t nr = strtoull(argv[2], nullptr, 10);
	uint64_t nq = strtoull(argv[3], nullptr, 10);
	uint64_t np = strtoull(argv[4], nullptr, 10);

    //LSB_Init("sequential", 0);

    double* a = 0;
	double* c4 = 0;
	double* sum = 0;

    //a = (double*) calloc()

	//init_array_seq(a_, c4, &sum, nr, nq, np);



    //LSB_Finalize();

    return 0;
}