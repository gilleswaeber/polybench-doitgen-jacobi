#include <doitgen.hpp>
#include <utils.hpp>
#include <chrono>
#include <omp.h>
#include <liblsb.h>
/*#include <liblsb.h>
#include <papi.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <iostream>*/

/*struct read_format {
	uint64_t nr;
	struct {
		uint64_t value;
		uint64_t id;
	} values[];
};

long perf_event_open(
	perf_event_attr* hw_event,
	pid_t pid,
	int cpu,
	int group_fd,
	unsigned long flags
) {
	int ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
	return ret;
}*/

#define RUNS 10

void init(double** a_in, double** a_out, double** c4, double** sum, uint64_t nr, uint64_t nq, uint64_t np) {
	*a_in = (double*)allocate_data(nr * nq * np, sizeof(double));
	*sum = (double*)allocate_data(nr * nq * np, sizeof(double));
	*c4 = (double*)allocate_data(np * np, sizeof(double));
	*a_out = (double*)allocate_data(nr * nq * np, sizeof(double));

	init_array(nr, nq, np, *a_in, *c4);
	memset(*a_out, 0.0, nr * nq * np * sizeof(double));
}

/*
* perf_event_attr attr;

	uint64_t id1, id2;
	// select what we want to count
	memset(&attr, 0, sizeof(perf_event_attr));
	attr.size = sizeof(perf_event_attr);
	attr.type = PERF_TYPE_RAW;
	attr.config = 0x40c7;
	attr.disabled = 1;
	attr.exclude_kernel = 1; // do not count the instruction the kernel executes
	attr.exclude_hv = 1;
	attr.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
	attr.inherit = 0;

	// open a file descriptor
	int fd_flops = perf_event_open(&attr, 0, -1, -1, 0);

	ioctl(fd_flops, PERF_EVENT_IOC_ID, &id1);

	attr.config = PERF_COUNT_HW_REF_CPU_CYCLES;
	attr.type = PERF_TYPE_HARDWARE;
	int fd_cycles = perf_event_open(&attr, 0, -1, fd_flops, 0);
	ioctl(fd_cycles, PERF_EVENT_IOC_ID, &id2);



	if (fd_flops == -1 || fd_cycles == -1)
	{
		// handle error
		printf("ERROR\n");
	}

	ioctl(fd_flops, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
	ioctl(fd_flops, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

	kernel_func

	ioctl(fd_flops, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);


	after

	char buf[4096];
	struct read_format* rf = (struct read_format*)buf;
	uint64_t count_flops, count_cycles;

	read(fd_flops, buf, sizeof(buf));
	for (uint64_t i = 0; i < rf->nr; i++) {
		if (rf->values[i].id == id1) {
			count_flops = rf->values[i].value;
		}
		else if (rf->values[i].id == id2) {
			count_cycles = rf->values[i].value;
		}
	}



	std::cout << "Count : " << count_flops << std::endl;
	std::cout << "Cycles : " << count_cycles << std::endl;
	// count now has the (approximated) result

	// close the file descriptor
	close(fd_flops);
	close(fd_cycles);
*/

//To change to 7 for the cluster
#define THREADS_SIZES 5
const static int threads[] = { 1, 2, 4, 8, 16, 32, 48 };

#define BLOCKING_WINDOW_SIZES 6
const static uint64_t blocking_windows[] = { 8, 16, 32, 64, 128, 256 };

void transpose(double* src, double* dst, uint64_t N, const int M) {
#pragma omp parallel for
	for (uint64_t n = 0; n < N * M; n++) {
		uint64_t i = n / N;
		uint64_t j = n % N;
		dst[n] = src[M * j + i];
	}
}

int main() {
	uint64_t nr = 128;
	uint64_t nq = 512;
	uint64_t np = 512;

	double* a_in;
	double* a_out;
	double* c4;
	double* sum;
	init(&a_in, &a_out, &c4, &sum, nr, nq, np);

	MPI::Init();

	LSB_Init("doitgen-openMP", 0);

	LSB_Set_Rparam_long("NR", nr);
	LSB_Set_Rparam_long("NQ", nq);
	LSB_Set_Rparam_long("NP", np);

	/*
	* Benchmark with the optimal doitgen version.
	*/
	LSB_Set_Rparam_string("benchmark", "doitgen-optimal");
	for (uint64_t i = 0; i < THREADS_SIZES; ++i) {
		omp_set_num_threads(threads[i]);
		LSB_Set_Rparam_int("threads", threads[i]);
		for (uint64_t j = 0; j < RUNS; ++j) {
			LSB_Res();
			kernel_doitgen_no_blocking(nr, nq, np, a_in, a_out, c4, sum);
			LSB_Rec(j);
			memset(a_out, 0.0, nr * nq * np * sizeof(double));
		}
	}

	/*
	* Benchmark with the default sequential doitgen version.
	*/
	LSB_Set_Rparam_string("benchmark", "doitgen-seq");
	LSB_Set_Rparam_int("threads", threads[0]);
	for (uint64_t i = 0; i < RUNS; ++i) {
		LSB_Res();
		kernel_doitgen_seq(nr, nq, np, a_in, c4, sum);
		LSB_Rec(i);
		memset(a_in, 0, nr * nq * np * sizeof(double));
		memset(c4, 0, np * np * sizeof(double));
		init_array(nr, nq, np, a_in, c4);
	}

	/*
	* Benchmark with the parallel default doitgen version.
	*/
	LSB_Set_Rparam_string("benchmark", "doitgen-parallel-default");
	cleanup(sum);

	for (uint64_t i = 0; i < THREADS_SIZES; ++i) {
		omp_set_num_threads(threads[i]);
		LSB_Set_Rparam_int("threads", threads[i]);
		for (uint64_t j = 0; j < RUNS; ++j) {
			sum = (double*)allocate_data(np * threads[i], sizeof(double));
			LSB_Res();
			kernel_doitgen_openmp(nr, nq, np, a_in, c4, sum);
			LSB_Rec(j);
			memset(a_in, 0, nr * nq * np * sizeof(double));
			memset(c4, 0, np * np * sizeof(double));
			init_array(nr, nq, np, a_in, c4);
			cleanup(sum);
		}
	}

	/*
	* Benchmark with the transpose version.
	* In this version, the cost of the transposition is not counted.
	*/
	LSB_Set_Rparam_string("benchmark", "doitgen-parallel-transpose-not-counted");
	double* c4_transposed = (double*)allocate_data(np * np, sizeof(double));
	transpose(c4, c4_transposed, np, np);

	for (uint64_t i = 0; i < THREADS_SIZES; ++i) {
		omp_set_num_threads(threads[i]);
		LSB_Set_Rparam_int("threads", threads[i]);
		for (uint64_t j = 0; j < RUNS; ++j) {
			LSB_Res();
			kernel_doitgen_transpose(nr, nq, np, a_in, a_out, c4_transposed, sum);
			LSB_Rec(j);
			memset(a_out, 0, nr * nq * np * sizeof(double));
		}
	}

	/*
	* Benchmark with the transpose version.
	* In this version, the cost of the transposition is counted.
	*/
	LSB_Set_Rparam_string("benchmark", "doitgen-parallel-transpose-counted");
	memset(c4_transposed, 0.0, np * np * sizeof(double));

	for (uint64_t i = 0; i < THREADS_SIZES; ++i) {
		omp_set_num_threads(threads[i]);
		LSB_Set_Rparam_int("threads", threads[i]);
		for (uint64_t j = 0; j < RUNS; ++j) {
			LSB_Res();
			transpose(c4, c4_transposed, np, np);
			kernel_doitgen_transpose(nr, nq, np, a_in, a_out, c4_transposed, sum);
			LSB_Rec(j);
			memset(c4_transposed, 0, np * np * sizeof(double));
			memset(a_out, 0, nr * nq * np * sizeof(double));
		}
	}

	cleanup(c4_transposed);

	/*
	* Benchmark with the blocking version.
	*/
	LSB_Set_Rparam_string("benchmark", "doitgen-seq-blocking");
	LSB_Set_Rparam_int("threads", threads[0]);
	for (uint64_t i = 0; i < BLOCKING_WINDOW_SIZES; ++i) {
		for (uint64_t j = 0; j < RUNS; ++j) {
			LSB_Res();
			kernel_doitgen_blocking(nr, nq, np, a_in, a_out, c4, sum, blocking_windows[i]);
			LSB_Rec(j);
			memset(a_out, 0, nr * nq * np * sizeof(double));
		}
	}
	



	LSB_Finalize();
	MPI::Finalize();

	cleanup(a_in);
	cleanup(a_out);
	cleanup(c4);
	//cleanup(sum);
	return 0;
}