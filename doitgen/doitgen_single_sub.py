
import argparse
import os
from pathlib import Path
from typing_extensions import runtime

#'basic',
#'transpose'


bench_types = [
    'basic',
    'transpose'
]

"""
bench_types = [
    'write_1',
    'write_2',
    'write_3',
    'write_4'
]
"""

proc_model = "EPYC_7H12" # "XeonE3_1585Lv5" #"XeonE3_1585Lv5" #XeonE3_1585Lv5 #EPYC_7H12

#cluster_outputs_locations = "/cluster/scratch/qguignard/"

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

def get_command(num_cores, out_file, bench_type, processor_model, run_index, nr, nq, np): 
    return "mpirun -np " + str(num_cores) + " ./dphpc-doitgen-mpi-benchmark " + out_file + " " + bench_type + " " + processor_model + " " + str(run_index) + " " + str(nr) + " " + str(nq) + " " + str(np)

def get_sequential_command(processor_model, run_index, nr, nq, np):
    return "./dphpc-doitgen-sequential-benchmark " + processor_model + " " + str(run_index) + " " + str(nr) + " " + str(nq) + " " + str(np)

def get_rm_file_command(path):
    return f"rm {path}" + "\n"

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
    parser.add_argument('--output_location',
                        help='outputpath',
                        default="/cluster/scratch/qguignard/",
                        type=str)
    parser.add_argument('--is_home',
                        help='default 0 for cluster 1 for laptop',
                        default=0,
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
    return f'-R "select[model=={model}]" -R "span[ptile=48]" '

def main():

    args = parse_args()

    cores = range(0, args.n + 1, 2)
    result = "#!/bin/bash \n" 

    if (args.is_home == 0):
        result += "bsub -n 48 -W 48:00 " + get_proc_selection(proc_model) + " <<EOF " +'\n'
    
    for i in range(args.runs):
        result += get_sequential_command(proc_model, i, args.nr, args.nq, args.np) + "\n"

    output_location = args.output_location if (args.is_home == 0) else "data/"

    index = 0
    for bench_type in bench_types:
        for c in cores:
            for i in range(args.runs):
                if (c == 0): # for the 0 cores
                    c = 1
                output_path = output_location + args.output + proc_model + "_" + str(index)
                result += get_command(c, output_path, bench_type, proc_model, i, c * args.nr, args.nq, args.np) + "\n"
                result += get_rm_file_command(output_path)
                index += 1
    if (args.is_home == 0):
        result += "EOF\n"

    print(result)
    try:
        create_file_at(result)
    except:
        print("failed to create the file")

if __name__ == "__main__":
    main()