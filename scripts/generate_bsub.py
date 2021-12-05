RUNS = 10
threads = [1, 2, 4, 8, 16, 32, 48]
benchmarks = [
    "polybench",
    "polybench_parallel",
    "polybench_parallel_local_sum",
    "transpose",
    # "blocking",
    "inverted_loop",
    # "inverted_loop_blocking",
    "inverted_loop_avx2",
]

NR = 512
NQ = 512
NP = 512

BLOCKING_IDX = 6

windows = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512]


def main():
    result = ""
    for i in range(len(benchmarks)):
        for j in range(len(threads)):
            """if i == BLOCKING_IDX:
                for k in range(len(windows)):
                    for l in range(RUNS):
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
            else:"""
            for k in range(RUNS):
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
                    + str(k)
                    + "\n"
                )
    script_file = open("bsub_openMP.sh", "w")
    script_file.write(result)
    script_file.close()


if __name__ == "__main__":
    main()
