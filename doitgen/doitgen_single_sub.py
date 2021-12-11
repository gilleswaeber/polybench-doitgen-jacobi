
import argparse
import os
from pathlib import Path


bench_types = [
    'basic'
]

proc_model = "EPYC_7H12"


# This scripts generates a submission script for euler in order to run all the
# benchmarks of doitgen mpi version

name = "doitgen_launch_benchmark.sh"

copy_sh_to = "copy_benchmark_to_cluster.sh"
copy_sh_from = "get_results.sh"

copy_local_to_remote = "scp " + name + " qguignard@euler.ethz.ch:dphpc-project/build/doitgen/benchmark"
copy_remote_to_local = """
now=$(date +%m%d%H%M)
mkdir results_${now}
mkdir lsfs_${now}
scp qguignard@euler.ethz.ch:dphpc-project/build/doitgen/benchmark/lsb* ./results_${now}
scp qguignard@euler.ethz.ch:dphpc-project/build/doitgen/benchmark/lsf* ./lsfs_${now}
"""

def get_command(num_cores, out_file, bench_type, nr, nq, np): 
    return "mpirun -np " + str(num_cores) + " ./dphpc-doitgen-mpi-benchmark " + out_file + " " + bench_type + " " + str(nr) + " " + str(nq) + " " + str(np)

def get_sequential_command(nr, nq, np):
    return "./dphpc-doitgen-sequential-benchmark " + str(nr) + " " + str(nq) + " " + str(np)

def parse_args():

    parser = argparse.ArgumentParser(description='Generate bsub for doitgen')
    
    # training
    parser.add_argument('--runs',
                        help='number of runs per benchmark',
                        default=10,
                        type=int)
    parser.add_argument('--n',
                        help='number of cores on the cluster /also processes',
                        default=48,
                        type=int)
    parser.add_argument('--output',
                        help='name of the output',
                        default="mpi.out",
                        type=str)
    parser.add_argument('--nr',
                        help='size of nr',
                        default=128,
                        type=int)
    parser.add_argument('--nq',
                        help='size of nq',
                        default=512,
                        type=int)
    parser.add_argument('--np',
                        help='size of np',
                        default=512,
                        type=int)
    
    args = parser.parse_args()

    return args


def create_file_at(result):

    path = Path(".")
    root = path.parent.absolute()
    doitgen_benchmark_path = root / "build" / "doitgen" / "benchmark"
    doitgen_benchmark_path_str = str(doitgen_benchmark_path.resolve())

    doitgen_benchmark_bench_sh_path = doitgen_benchmark_path_str + "/" + name
    doitgen_benchmark_copy_from_sh_path = doitgen_benchmark_path_str + "/" + copy_sh_to
    doitgen_benchmark_copy_to_sh_path = doitgen_benchmark_path_str + "/" + copy_sh_from

    f = open(doitgen_benchmark_bench_sh_path, "w")
    f.write(result)
    f.close()

    copy_local_to_remote
    f = open(doitgen_benchmark_copy_from_sh_path, "w")
    f.write(copy_local_to_remote)
    f.close()

    copy_local_to_remote
    f = open(doitgen_benchmark_copy_to_sh_path, "w")
    f.write(copy_remote_to_local)
    f.close()


def get_proc_selection(model):
    return f'-R "select[model=={model}]"'

def main():

    args = parse_args()

    cores = range(0, args.n + 1, 2)
    result = "bsub -n 48 " + get_proc_selection(proc_model) + '\n'

    result += get_sequential_command(args.nr, args.nq, args.np) + "\n"

    index = 0
    for bench_type in bench_types:
        for c in cores:
            for i in range(args.runs):
                if (c == 0): # for the 0 cores
                    c = 1
                result += get_command(c, "/scratch/" + args.output + str(index), bench_type, c * args.nr, args.nq, args.np) + "\n"
                index += 1

    print(result)
    try:
        create_file_at(result)
    except:
        print("failed to create the file")

if __name__ == "__main__":
    main()