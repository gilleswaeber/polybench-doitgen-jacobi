RUNS = 10


def main():
    result_total = ""
    for i in range(RUNS):
        result_total += "../dphpc-bsub-benchmark " + str(i) + "\n"
    script_file = open("bsub_benchmark_one_bsub.sh", "w")
    script_file.write(result_total)
    script_file.close()

    result_total = ""
    for i in range(RUNS):
        result_total += (
            'bsub -n 1 -R "select[model==EPYC_7H12]" -R "rusage[mem=3072]" ../dphpc-bsub-benchmark '
            + str(i)
            + "\n"
        )

    script_file = open("bsub_benchmark_bsubs.sh", "w")
    script_file.write(result_total)
    script_file.close()


if __name__ == "__main__":
    main()
