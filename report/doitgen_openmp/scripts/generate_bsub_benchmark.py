RUNS = 10


def generate_individual(bench_name):

    result_individual = ""
    for i in range(RUNS):
        result_individual += f"./dphpc-bsub-benchmark {i} {bench_name} batched\n"
    
    return result_individual

def generate_batch(bench_name):
    result_total = ""
    for i in range(RUNS):
        result_total += (
            f'bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark {i} {bench_name} single\n'
        )

    return result_total

def main():

    result_individual = 'bsub -n 48 -W 24:00 -R "select[model==EPYC_7H12]" << EOF\n'
    result_batch = ""

    result_individual += generate_individual("sequential")
    result_individual += generate_individual("random")
    result_individual += "EOF\n"

    result_batch += generate_batch("sequential")
    result_batch += generate_batch("random")

    #script_file = open("bsub_benchmark_one_bsub.sh", "w")
    #script_file.write(result_individual)
    #script_file.close()

    result_individual += result_batch    
    script_file = open("bsub_benchmark_rand_and_seq.sh", "w")
    script_file.write(result_individual)
    script_file.close()

if __name__ == "__main__":
    main()
