#include <doitgen.hpp>
#include <utils.hpp>
#include <liblsb.h>
#include <papi.h>

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

	float real_time, proc_time, mflops;
	long long flpops;
	float ireal_time, iproc_time, imflops;
	long long iflpops;
	int retval;

	if((retval = PAPI_flops_rate(PAPI_FP_OPS, &ireal_time, &iproc_time, &iflpops, &imflops)) < PAPI_OK)
	{
		printf("Error : %s", PAPI_strerror(retval));
		printf("Could not initialise PAPI_flops \n");
		printf("Your platform may not support floating point operation event.\n");
		printf("retval: %d\n", retval);
		exit(1);
	}

	kernel_doitgen_no_blocking(nr, nq, np, a_in, a_out, c4, sum);

	if ((retval = PAPI_flops_rate(PAPI_FP_OPS, &real_time, &proc_time, &flpops, &mflops)) < PAPI_OK)
	{
		printf("retval: %d\n", retval);
		exit(1);
	}
	
	printf("Real_time: %f Proc_time: %f flpops: %lld MFLOPS: %f\n",
		real_time, proc_time, flpops, mflops);

	cleanup(a_in);
	cleanup(a_out);
	cleanup(c4);
	cleanup(sum);
	return 0;
}