
import argparse
import os
from pathlib import Path

# This scripts generates a submission script for euler in order to run all the
# benchmarks of doitgen mpi version

name = "doitgen_launch_benchmark.sh"

copy_sh_to = "copy_benchmark_to_cluster.sh"
copy_sh_from = "get_results.sh"

copy_local_to_remote = "scp " + name + " qguignard@euler.ethz.ch:dphpc-project/build/doitgen/benchmark"
copy_remote_to_local = "scp qguignard@euler.ethz.ch:dphpc-project/build/doitgen/benchmark/* ."


def get_sub(num_cores, out_file, nr, nq, np): 
    return "bsub -n " + str(num_cores) + " mpirun -np " + str(num_cores) + " ./dphpc-doitgen-mpi-benchmark " + out_file + " " + str(nr) + " " + str(nq) + " " + str(np)

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

def main():

    args = parse_args()

    cores = range(0, args.n + 1, 2)
    result = ""
    index = 0
    for c in cores:
        for i in range(args.runs):
            if (c == 0): # for the 0 cores
                c = 1
            result += get_sub(c, "/scratch/" + args.output + str(index), c * args.nr, args.nq, args.np) + "\n"
            index += 1

    print(result)
    try:
        create_file_at(result)
    except:
        print("failed to create the file")

if __name__ == "__main__":
    main()