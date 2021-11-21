#include <doitgen.hpp>
#include <utils.hpp>
#include <liblsb.h>
#include <papi.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <iostream>

struct read_format {
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
}

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

	perf_event_attr attr;

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

	// open a file descriptor
	int fd_flops = perf_event_open(&attr, 0, -1, -1, 0);

	ioctl(fd_flops, PERF_EVENT_IOC_ID, &id1);


	attr.config = 0x003c;
	int fd_cycles = perf_event_open(&attr, 0, -1, fd_flops, 0);
	ioctl(fd_cycles, PERF_EVENT_IOC_ID, &id2);



	if (fd_flops == -1 ||fd_cycles == -1)
	{
		// handle error
		printf("ERROR\n");
	}

	ioctl(fd_flops, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
	ioctl(fd_flops, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

	kernel_doitgen_no_blocking(nr, nq, np, a_in, a_out, c4, sum);

	ioctl(fd_flops, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);

	/*long long count_flops;
	read(fd_flops, &count_flops, sizeof(long long));

	long long count_cycles;
	read(fd_cycles, &count_flops, sizeof(long long));*/

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

	cleanup(a_in);
	cleanup(a_out);
	cleanup(c4);
	cleanup(sum);
	return 0;
}