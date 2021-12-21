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

benchmarks = [
    "transpose",
    "inverted_loop",
    "inverted_loop_avx2",
    "transpose_local_sum",
]

benchmarks_blocking = ["inverted_loop_blocking"]

benchmarks_scaling = [
    "inverted_loop",
    "inverted_loop_avx2",
    "transpose_local_sum",
]

NR = 512
NQ = 512
NP = 512

windows = [8, 16, 32, 64, 128, 256, 512]


def main():
    result_clasic = ""
    for i in range(len(benchmarks)):
        for j in range(len(threads)):
            for k in range(RUNS):
                result_clasic += (
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
    result_blocking = ""
    for i in range(len(benchmarks_blocking)):
        for j in range(len(threads)):
            for k in range(len(windows)):
                for l in range(RUNS):
                    result_blocking += (
                        "../dphpc-doitgen-openmp-benchmark "
                        + benchmarks_blocking[i]
                        + " "
                        + str(NR)
                        + " "
                        + str(NQ)
                        + " "
                        + str(NP)
                        + " "
                        + str(threads[j])
                        + " "
                        + str(l)
                        + " "
                        + str(windows[k])
                        + "\n"
                    )
    result_scaling = ""
    for i in range(len(benchmarks_scaling)):
        for j in range(len(threads_scaling)):
            for k in range(RUNS):
                result_scaling += (
                    "../dphpc-doitgen-openmp-benchmark "
                    + benchmarks_scaling[i]
                    + " "
                    + str(128 * threads_scaling[j])
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
    script_file = open("bsub_classic.sh", "w")
    script_file.write(result_clasic)
    script_file.close()

    script_file = open("bsub_blocking.sh", "w")
    script_file.write(result_blocking)
    script_file.close()

    script_file = open("bsub_scaling.sh", "w")
    script_file.write(result_scaling)
    script_file.close()


if __name__ == "__main__":
    main()
