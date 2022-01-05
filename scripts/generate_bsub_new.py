RUNS = 10
threads = [1, 2, 4, 8, 16, 32, 48]

threads_matrix = [8, 16, 32, 48]

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

matrix_scale = [512, 1024, 1536, 2048]

blocking_scale = [2048]

benchmarks = [
    "transpose",
    "inverted_loop",
    "inverted_loop_avx2",
    "inverted_loop_local_sum",
    "transpose_local_sum",
    "polybench_parallel_local_sum",
    "inverted_loop_local_sum_1D",
    "inverted_loop_avx2_local_sum_1D",
    "inverted_loop_avx2_local_sum"
]

benchmarks_blocking = ["inverted_loop_blocking"]

benchmarks_scaling = [
    "transpose",
    "inverted_loop",
    "inverted_loop_avx2",
    "inverted_loop_local_sum",
    "transpose_local_sum",
    "polybench_parallel_local_sum",
    "inverted_loop_local_sum_1D",
    "inverted_loop_avx2_local_sum_1D",
    "inverted_loop_avx2_local_sum"
]

benchmarks_matrix_scale = ["inverted_loop_avx2",
"inverted_loop_avx2_local_sum"
]

benchmarks_local_sum_1D = [
    "inverted_loop_local_sum_1D",
    "inverted_loop_avx2_local_sum_1D",
]

benchmarks_optimized = [
    "inverted_loop_avx2_blocking",
    "inverted_loop_avx2",
]

NR = 512
NQ = 512
NP = 512

windows = [8, 16, 32, 64, 128, 256, 512, 1024]


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
                for l in range(len(blocking_scale)):
                    for m in range(RUNS):
                        result_blocking += (
                            "../dphpc-doitgen-openmp-benchmark "
                            + benchmarks_blocking[i]
                            + " "
                            + str(256)
                            + " "
                            + str(blocking_scale[l])
                            + " "
                            + str(blocking_scale[l])
                            + " "
                            + str(threads[j])
                            + " "
                            + str(m)
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

    result_1D = ""
    for i in range(len(benchmarks_local_sum_1D)):
        for j in range(len(threads)):
            for k in range(RUNS):
                result_1D += (
                    "../dphpc-doitgen-openmp-benchmark "
                    + benchmarks_local_sum_1D[i]
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
    result_matrix_scale = ""
    for i in range(len(benchmarks_matrix_scale)):
        for j in range(len(threads)):
            for k in range(len(matrix_scale)):
                for l in range(RUNS):
                    result_matrix_scale += (
                        "../dphpc-doitgen-openmp-benchmark "
                        + benchmarks_matrix_scale[i]
                        + " "
                        + str(256)
                        + " "
                        + str(matrix_scale[k])
                        + " "
                        + str(matrix_scale[k])
                        + " "
                        + str(threads[j])
                        + " "
                        + str(l)
                        + "\n"
                    )

    result_optimized = ""
    for i in range(len(benchmarks_optimized)):
            for k in range(len(windows)):
                    for m in range(RUNS):
                        result_optimized += (
                            "../dphpc-doitgen-openmp-benchmark "
                            + benchmarks_optimized[i]
                            + " "
                            + str(NR)
                            + " "
                            + str(2048)
                            + " "
                            + str(2048)
                            + " "
                            + str(48)
                            + " "
                            + str(m)
                            + " "
                            + str(windows[k])
                            + "\n"
                        )

    script_file = open("bsub_optimized.sh", "w")
    script_file.write(result_optimized)
    script_file.close()

    script_file = open("bsub_classic.sh", "w")
    script_file.write(result_clasic)
    script_file.close()

    script_file = open("bsub_matrix_scale.sh", "w")
    script_file.write(result_matrix_scale)
    script_file.close()

    script_file = open("bsub_blocking.sh", "w")
    script_file.write(result_blocking)
    script_file.close()

    script_file = open("bsub_scaling.sh", "w")
    script_file.write(result_scaling)
    script_file.close()

    """script_file = open("bsub_1D.sh", "w")
    script_file.write(result_1D)
    script_file.close()"""


if __name__ == "__main__":
    main()
