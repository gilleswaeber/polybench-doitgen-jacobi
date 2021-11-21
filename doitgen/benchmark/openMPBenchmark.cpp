#include <doitgen.hpp>
#include <utils.hpp>
#include <liblsb.h>

#define RUNS 10

void init(double** a_in, double** a_out, double** c4, double** sum, uint64_t nr, uint64_t nq, uint64_t np) {
	*a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	*sum = (double*)allocate_data(nr * nq * np, sizeof(double));
	*c4 = (double*)allocate_data(np * np, sizeof(double));
	*a_out = (double*)allocate_data(nr * nq * np, sizeof(double));

	init_array(nr, nq, np, *a_in, *c4);
	memset(*a_out, 0.0, nr * nq * np);
}

int main() {
	uint64_t nr = 512;
	uint64_t nq = 512;
	uint64_t np = 512;

	double* a_in;
	double* a_out;
	double* c4;
	double* sum;
	init(&a_in, &a_out, &c4, &sum, nr, nq, np);
	MPI::Init();

	LSB_Init("doitgen-openMP-benchmark", 0);

	kernel_doitgen_no_blocking(nr, nq, np, a_in, a_out, c4, sum);


	LSB_Finalize();
	cleanup(a_in);
	cleanup(a_out);
	cleanup(c4);
	cleanup(sum);
	return 0;
}