import argparse
import math
import re
import sys
from itertools import chain
from pathlib import PosixPath
from typing import List, TextIO, Sequence

"""
Generate the submission bash script to benchmark Jacobi MPI for the ETH cluster

It relies on the mpi_time.sh script to set the environment variables for libLSB and move the lsb files to the correct
location.

Author: Gilles Waeber
"""

DEFAULT_PROGRAM_PATH = './build/jacobi/benchmark'
DEFAULT_COLLECTOR = 'python ./jacobi/jacobi_mpi_collect.py'
SCRIPT_HEADER = '''#!/bin/bash
# Jacobi MPI benchmark submission script
set -euo pipefail

module purge  # clear active modules
module load gcc/9.3.0 openmpi/4.0.2 python/3.6.5 cmake/3.20.3
set -x
now=$(date +%m%d%H%M%S)
'''
GLOBAL_SCRATCH_DIR = '$SCRATCH'
NODE_SCRATCH_DIR = r'\$TMPDIR'


def read_range(spec: str) -> Sequence[int]:
    if re.match(r'^\d+$', spec):
        return [int(spec)]
    else:
        parts = spec.split(':')
        values = []
        for p in parts:
            if re.match(r'^\d+$', p):
                if len(values):
                    values = values[::int(p)]
                else:
                    values = [int(p)]
            elif re.match(r'^\d+-\d+$', p):
                if len(values):
                    raise ValueError('A range must be the first element')
                a, b = p.split('-')
                values = range(int(a), int(b) + 1)
            elif p == 'sqr':
                if not len(values):
                    raise ValueError('Cannot use on an empty list')
                values = [v**2 for v in values]
            elif p == '2pow':
                if not len(values):
                    raise ValueError('Cannot use on an empty list')
                values = [2**v for v in values]
            else:
                raise ValueError(f'Invalid element {p}')
        if not len(values):
            raise ValueError(f'Empty range')
        return values

def parse_args():
    parser = argparse.ArgumentParser(description='Generate Euler script for jacobi-1d')
    parser.add_argument('programs',
                        help='program(s) to benchmark',
                        nargs='+')
    parser.add_argument('--program-path',
                        help=f'location of the binaries (default: {DEFAULT_PROGRAM_PATH})',
                        default=DEFAULT_PROGRAM_PATH)
    parser.add_argument('--collector',
                        help=f'program to run at the end to collect the results for the path passed as a parameter (default: {DEFAULT_COLLECTOR})',
                        default=DEFAULT_COLLECTOR)
    parser.add_argument('--runs',
                        help='number of runs per benchmark',
                        default=10,
                        type=int)
    parser.add_argument('--cores', dest='total_cores',
                        help='number of cores to request on the cluster',
                        default=48,
                        type=int)
    parser.add_argument('--np', dest='cores',
                        help='number of MPI processes to use for the benchmark, '
                             'use 1-10 for values 1 to 10 (inclusive), 2-8:2 for 2, 4, 6, 8, and 2-4:sqr for 4, 9, 16, '
                             'default: 1-#cores',
                        nargs='+',
                        type=read_range)
    parser.add_argument('--output',
                        help='output (default: stdout)',
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
                        nargs="+", default=[[8]],
                        type=read_range)
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
    parser.add_argument('--node-scratch',
                        help='use node scratch (specify memory in MB)',
                        type=int,
                        default=None)
    parser.add_argument('--nodes',
                        help='number of nodes to request on the cluster',
                        type=int, default=1)
    parser.add_argument('--cpu',
                        help='force the use of a specfic CPU, list them with lsinfo -m')
    args = parser.parse_args()
    args.cores = list(sorted(set(chain(*args.cores))))
    args.ghost_cells = list(sorted(set(chain(*args.ghost_cells))))
    return args


def generate_jacobi_mpi_sub(*, programs: List[str], program_path: str, collector: str, runs: int, total_cores: int,
                            cores: List[int], output: TextIO, n: int, time_steps: int, ghost_cells: List[int],
                            do_cleanup: bool, do_final: bool, send_email: bool,
                            minutes: int, cd: str = None, node_scratch: int = None, nodes=1, cpu: str = None):
    assert runs >= 1
    output_dir = "output/$now"
    if not len(cores):
        cores = range(1, total_cores + 1)
    else:
        max_cores = max(cores)
        if max_cores > total_cores:
            raise ValueError("Oversubscription is not supported on the cluster")
        elif max_cores < total_cores:
            if max_cores % nodes == 0:
                total_cores = max_cores
                print(f"Requested cores reduced to {total_cores}", file=sys.stderr)
            else:
                needed_cores = (max_cores // nodes + 1) * nodes
                if needed_cores != total_cores:
                    total_cores = needed_cores
                    print(f"Requested cores reduced to {total_cores} "
                          f"({max_cores} needed over {nodes} nodes)", file=sys.stderr)
    scratch_dir = GLOBAL_SCRATCH_DIR
    if node_scratch is not None:
        assert nodes == 1
        scratch_dir = NODE_SCRATCH_DIR
    with output as f:
        f.write(SCRIPT_HEADER)
        if cd is not None:
            f.write(f"""cd "{cd}"\n""")
        f.write(f"mkdir -p {output_dir}\n")
        f.write(f"chmod u+x mpi_time.sh\n")
        bsub_job_name = f"{programs[0]}_n{n}t{time_steps}"
        bsub_flags = f"-n {total_cores} -o {output_dir}'/job' -J {bsub_job_name} -W {minutes}"
        # reserved scratch disk space in MB, multiplied by the number of cores
        if node_scratch is not None:
            bsub_flags += f" -R 'rusage[scratch={node_scratch}]'"
        if nodes != 1:
            assert total_cores % nodes == 0
            ptile = total_cores // nodes
            bsub_flags += f" -R 'span[ptile={ptile}]'"
        if cpu is not None:
            bsub_flags += f" -R 'select[model=={cpu}]'"
        if send_email:
            bsub_flags += " -N"
        f.write(f"bsub {bsub_flags} <<EOF\n")
        f.write(f"echo 'Script arguments: {' '.join(sys.argv[1:])}'\n")
        f.write(f"echo '  np: {','.join(str(c) for c in cores)}, ghost: {','.join(str(g) for g in ghost_cells)}'\n")
        f.write(f"set -eux\n")
        for i in range(runs):  # runs in the outer loop to improve stability
            for program in programs:
                executable = str(PosixPath(program_path) / program)
                if '/' not in executable:
                    executable = f"./{executable}"
                for np in reversed(cores):
                    for g in ghost_cells:
                        job_name = f"{program}_n{n}t{time_steps}g{g}c{np}"
                        job_log = f"{output_dir}/{job_name}"
                        dest = f"{scratch_dir}/${{now}}_{job_name}_r{i}"
                        time_cmd = f"mpi_time.sh {job_log}r{i}t"
                        mpi_flags = f"-np {np}"
                        if nodes > 1:
                            mpi_flags += " --map-by node"
                        # if np > total_cores:  # not supported by the cluster
                        #    mpi_flags = f"-oversubscribe {mpi_flags}"
                        job = f"mpirun {mpi_flags} {time_cmd} {executable} {n} {time_steps} {g} {dest}\n"
                        f.write(job)
                        if do_cleanup:
                            f.write(f"rm {dest}\n")
        if do_final:
            final_job = f"{collector} {output_dir}\n"
            f.write(final_job)

        f.write("EOF\n")


if __name__ == "__main__":
    generate_jacobi_mpi_sub(**vars(parse_args()))
