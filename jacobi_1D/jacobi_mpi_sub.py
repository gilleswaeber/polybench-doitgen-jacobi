import argparse
import math
from typing import List, TextIO

import sys

EXECUTABLE = './build/jacobi_1D/benchmark/dphpc-jacobi-mpi-benchmark'
final_executable = 'python ./jacobi_1D/jacobi_mpi_collect.py'
script_header = '''#!/bin/bash
# Jacobi 1D submission script
set -euo pipefail

module purge  # clear active modules
#module load new gcc/6.3.0 open_mpi/3.0.0 cmake/3.13.5 python/3.7.1  # old stack
module load gcc/9.3.0 openmpi/4.0.2 python/3.6.5 cmake/3.20.3
set -x
now=$(date +%m%d%H%M%S)
'''
lsb_setup = '''
LSB_OUTPUT_FORMAT=efficient
export LSB_OUTPUT_FORMAT
'''
script_footer = ''
GLOBAL_SCRATCH_DIR = '$SCRATCH'
NODE_SCRATCH_DIR = r'\$TMPDIR'


def parse_args():
    parser = argparse.ArgumentParser(description='Generate Euler script for jacobi-1d')
    # training
    parser.add_argument('--runs',
                        help='number of runs per benchmark',
                        default=10,
                        type=int)
    parser.add_argument('--cores', dest='total_cores',
                        help='number of cores on the cluster /also processes',
                        default=48,
                        type=int)
    parser.add_argument('--cores-step', dest='cores_step',
                        help='steps to take when testing number of cores (default: 2)',
                        default=2,
                        type=int)
    parser.add_argument('--output',
                        help='name of the output',
                        default='-',
                        type=argparse.FileType('wt', encoding='utf-8'))
    parser.add_argument('--n',
                        help='base size of the array, multiplied by the number of cores',
                        default=1_000_000,
                        type=int)
    parser.add_argument('--time-steps',
                        help='number of iterations',
                        default=1_000,
                        type=int)
    parser.add_argument('--minutes',
                        help='time limit, in minutes',
                        default=4 * 60,
                        type=int)
    parser.add_argument('--ghost-cells',
                        help='number of ghost cells (halo size)',
                        nargs="+", default=[8],
                        type=int)
    parser.add_argument('--no-cleanup',
                        help='skip cleanup + final job generation',
                        action='store_false',
                        dest='do_cleanup')
    parser.add_argument('--no-final',
                        help='skip final job generation',
                        action='store_false',
                        dest='do_final')
    parser.add_argument('--no-email',
                        help='don\'t send an e-mail when done',
                        action='store_false',
                        dest='send_email')
    parser.add_argument('--dir',
                        help='change director to',
                        dest='cd')
    parser.add_argument('--alt',
                        help='use an alternative version of the executable',
                        dest='alternative')
    parser.add_argument('--lsb',
                        help='add environment variables and commands for libLSB',
                        action='store_true',
                        dest='with_lsb')
    parser.add_argument('--node-scratch',
                        help='use node scratch',
                        action='store_true')
    parser.add_argument('--nodes',
                        help='use multiple nodes',
                        type=int, default=1)
    parser.add_argument('--bsub-cpu',
                        help='force the use of a specfic CPU, list them with lsinfo -m')
    args = parser.parse_args()
    return args


def generate_jacobi_mpi_sub(*, runs: int, total_cores: int, cores_step: int, output: TextIO, n: int, time_steps: int,
                            ghost_cells: List[int], do_cleanup: bool, do_final: bool, send_email: bool, with_lsb: bool,
                            minutes: int, cd: str = None, alternative: str = None,
                            node_scratch=False, nodes=1, bsub_cpu: str = None):
    assert runs >= 1
    output_dir = "output/$now"
    executable = EXECUTABLE if alternative is None else f"{EXECUTABLE}-{alternative}"
    is_2d = (alternative is not None and '2d' in alternative)
    scratch_dir = GLOBAL_SCRATCH_DIR
    if node_scratch:
        assert nodes == 1
        scratch_dir = NODE_SCRATCH_DIR
    with output as f:
        f.write(script_header)
        if with_lsb:
            f.write(lsb_setup)
        if cd is not None:
            f.write(f"""cd "{cd}"\n""")
        f.write(f"mkdir -p {output_dir}\n")
        f.write(f"chmod u+x mpi_time.sh\n")
        if is_2d:
            num_cores = [i ** 2 for i in range(1, int(math.sqrt(total_cores)) + 1)]
            total_cores = num_cores[-1]
        else:
            num_cores = [1] + list(range(cores_step, total_cores + 1, cores_step))
        job_base_name = f"jacobi1d_n{n}t{time_steps}"
        bsub_flags = f"-n {total_cores} -o {output_dir}'/{job_base_name}J' -J {job_base_name} -W {minutes}"
        # reserved scratch disk space in MB, multiplied by the number of cores
        if node_scratch:
            if is_2d:
                bsub_flags += f" -R 'rusage[scratch={8 * n * n // 1_000_000 + 10}]'"
            else:
                bsub_flags += f" -R 'rusage[scratch={8 * n // 1_000_000 + 10}]'"
        if nodes != 1:
            assert total_cores % nodes == 0
            assert is_2d or cores_step % nodes == 0
            ptile = total_cores // nodes
            bsub_flags += f" -R 'span[ptile={ptile}]'"
        if bsub_cpu is not None:
            bsub_flags += f" -R 'select[model=={bsub_cpu}]'"
        if send_email:
            bsub_flags += " -N"
        f.write(f"bsub {bsub_flags} <<EOF\n")
        f.write(f"echo 'Script arguments: {' '.join(sys.argv[1:])}'\n")
        if is_2d:
            f.write(f"echo 'Jacobi2D detected'\n")
        f.write(f"set -x\n")
        for i in range(runs):  # runs in the outer loop to improve stability
            for cores in reversed(num_cores):
                for g in ghost_cells:
                    job_name = f"{job_base_name}g{g}c{cores}"
                    if alternative is not None:
                        job_name += f"v{alternative}"
                    job_log = f"{output_dir}/{job_name}"
                    dest = f"{scratch_dir}/${{now}}_{job_name}_r{i}"
                    time_cmd = f"mpi_time.sh {job_log}r{i}t"
                    job = f"mpirun --map-by node -np {cores} {time_cmd} {executable} {n} {time_steps} {g} {dest}\n"
                    f.write(job)
                    if do_cleanup:
                        f.write(f"rm {dest}\n")
            if with_lsb:
                f.write(f"mv -t {output_dir} lsb.*\n")

        if do_final:
            final_job = f"{final_executable} {output_dir}\n"
            f.write(final_job)

        f.write("EOF\n")

        f.write(script_footer)


if __name__ == "__main__":
    generate_jacobi_mpi_sub(**vars(parse_args()))
