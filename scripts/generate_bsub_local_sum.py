RUNS = 10
threads = [1, 2, 4, 8, 16, 32, 48]

threads_scaling = [
    1,
    2,
    4,
    6,
    8,
    10,
    12,
    14,
    16,
    18,
    20,
    22,
    24,
    26,
    28,
    30,
    32,
    34,
    36,
    38,
    40,
    42,
    44,
    46,
    48,
]

# benchmarks = ["inverted_loop_avx2_local_sum", "transpose_local_sum"]

benchmarks = ["inverted_loop_local_sum"]

benchmark_scaling = ["inverted_loop_avx2", "inverted_loop", "inverted_loop_local_sum"]

NR = 512
NQ = 512
NP = 512

NR_START = 128


def main():
    result_total = ""
    for i in range(len(benchmarks)):
        for j in range(len(threads)):
            for k in range(RUNS):
                result_total += (
                    "../dphpc-doitgen-openmp-benchmark "
                    + benchmarks[i]
                    + " "
                    + str(NR)
                    + " "
                    + str(NQ)
                    + " "
                    + str(NP)
                    + " "
                    + str(threads[j])
                    + " "
                    + str(k)
                    + "\n"
                )
    script_file = open("bsub_openMP_local_sum.sh", "w")
    script_file.write(result_total)
    script_file.close()

    result_scaling = ""
    for i in range(len(benchmark_scaling)):
        for j in range(len(threads_scaling)):
            for k in range(RUNS):
                result_scaling += (
                    "../dphpc-doitgen-openmp-benchmark "
                    + benchmark_scaling[i]
                    + " "
                    + str(NR_START * threads_scaling[j])
                    + " "
                    + str(NQ)
                    + " "
                    + str(NP)
                    + " "
                    + str(threads_scaling[j])
                    + " "
                    + str(k)
                    + "\n"
                )

    script_file = open("bsub_openMP_scaling.sh", "w")
    script_file.write(result_scaling)
    script_file.close()


if __name__ == "__main__":
    main()
