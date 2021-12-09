RUNS = 10
threads = [1, 2, 4, 8, 16, 32, 48]
benchmarks_seq = ["polybench"]

benchmarks_non_blocking = [
    "polybench_parallel",
    "polybench_parallel_local_sum",
    "transpose",
    "inverted_loop",
    "inverted_loop_avx2",
]

benchmarks_blocking = ["blocking", "inverted_loop"]

NR = 512
NQ = 512
NP = 512

BLOCKING_IDX = 6

windows = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512]

"""
result += (
                            "bsub -n "
                            + str(threads[j])
                            + " "
                            + "../dphpc-doitgen-openmp-benchmark "
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
                            + str(l)
                            + " "
                            + str(windows[l])
                            + "\n"
                        )
"""


def main():
    result_non_blocking = ""
    result_blocking = ""
    result_total = ""
    for i in range(len(benchmarks_seq)):
        for j in range(RUNS):
            result_total += (
                "../dphpc-doitgen-openmp-benchmark "
                + benchmarks_seq[i]
                + " "
                + str(NR)
                + " "
                + str(NQ)
                + " "
                + str(NP)
                + " "
                + str(1)
                + " "
                + str(j)
                + "\n"
            )
    for i in range(len(benchmarks_non_blocking)):
        for j in range(len(threads)):
            for k in range(RUNS):
                result_total += (
                    "../dphpc-doitgen-openmp-benchmark "
                    + benchmarks_non_blocking[i]
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
    for i in range(len(benchmarks_blocking)):
        for j in range(len(threads)):
            for k in range(len(windows)):
                for l in range(RUNS):
                    result_total += (
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
    script_file = open("bsub_openMP.sh", "w")
    script_file.write(result_total)
    script_file.close()

    """script_file = open("bsub_openMP_blocking.sh", "w")
    script_file.write(result_blocking)
    script_file.close()"""


if __name__ == "__main__":
    main()
