RUNS = 10
threads = [1, 2, 4, 8, 16, 32, 48]

threads_scaling = [
    1,
    4,
    8,
    12,
    16,
    20,
    24,
    28,
    32,
    36,
    40,
    44,
    48
]

benchmarks_seq = ["polybench"]

benchmarks = [
    "polybench_parallel",
    "polybench_swap"
]

N = 10000
N_scale = 100
TSTEPS = 1000


def main():
    result_total = ""
    for i in range(len(benchmarks_seq)):
        for j in range(RUNS):
            result_total += (
                    '../jacobi/benchmark/jacobi2d-openmp-benchmark '
                    + benchmarks_seq[i]
                    + " "
                    + str(N)
                    + " "
                    + str(TSTEPS)
                    + " "
                    + str(1)
                    + " "
                    + str(j)
                    + "\n"
            )
    for i in range(len(benchmarks)):
        for j in range(len(threads)):
            for k in range(RUNS):
                result_total += (
                        '../jacobi/benchmark/jacobi2d-openmp-benchmark '
                        + benchmarks[i]
                        + " "
                        + str(N)
                        + " "
                        + str(TSTEPS)
                        + " "
                        + str(threads[j])
                        + " "
                        + str(k)
                        + "\n"
                )
    script_file = open("bsub_jacobi_2D_openMP.sh", "w")
    script_file.write(result_total)
    script_file.close()

    result_scaling = ""
    for i in range(len(benchmarks_seq)):
        for j in range(len(threads_scaling)):
            for k in range(RUNS):
                result_scaling += (
                        '../jacobi/benchmark/jacobi2d-openmp-benchmark '
                        + benchmarks_seq[i]
                        + " "
                        + str(N_scale * threads_scaling[j])
                        + " "
                        + str(TSTEPS)
                        + " "
                        + str(threads_scaling[j])
                        + " "
                        + str(k)
                        + "\n"
                )
    for i in range(len(benchmarks)):
        for j in range(len(threads_scaling)):
            for k in range(RUNS):
                result_scaling += (
                        '../jacobi/benchmark/jacobi2d-openmp-benchmark '
                        + benchmarks[i]
                        + " "
                        + str(N_scale * threads_scaling[j])
                        + " "
                        + str(TSTEPS)
                        + " "
                        + str(threads_scaling[j])
                        + " "
                        + str(k)
                        + "\n"
                )
    script_file = open("bsub_jacobi_2D_openMP_scaling.sh", "w")
    script_file.write(result_scaling)
    script_file.close()

if __name__ == "__main__":
    main()
