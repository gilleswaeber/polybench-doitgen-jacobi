import argparse

executable = './build/jacobi_1D/benchmark/dphpc-jacobi-mpi-benchmark'
script_header = '''#!/bin/bash
# Jacobi 1D submission script
set -euo pipefail

module purge  # clear active modules
module load new gcc/6.3.0 open_mpi/3.0.0 cmake/3.13.5
set -x
now=$(date +%Y%m%d_%H%M%S_)
'''
script_footer = ''


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
    parser.add_argument('--output',
                        help='name of the output',
                        default='-',
                        type=argparse.FileType('wt', encoding='utf-8'))
    parser.add_argument('--n',
                        help='base size of the array, multiplied by the number of cores',
                        default=240_000,
                        type=int)
    parser.add_argument('--time-steps',
                        help='number of iterations',
                        default=1_000,
                        type=int)
    parser.add_argument('--time-limit',
                        help='time limit, in minutes',
                        default=5,
                        type=int)
    parser.add_argument('--ghost-cells',
                        help='number of ghost cells (halo size)',
                        default=8,
                        type=int)
    parser.add_argument('--no-cleanup',
                        help='skip cleanup command generation',
                        action='store_false',
                        dest='cleanup')
    parser.add_argument('--dir',
                        help='change director to')

    args = parser.parse_args()
    return args


def generate_jacobi_mpi_sub(*, runs, total_cores, output, n, time_steps, ghost_cells, cleanup, time_limit, dir=None):
    assert runs >= 1
    with output as f:
        f.write(script_header)
        if dir is not None:
            f.write(f'cd "{dir}"\n')
        num_cores = [1] + list(range(2, total_cores + 1, 2))
        for cores in num_cores:
            job_name = f"jacobi1d_n{n}t{time_steps}g{ghost_cells}c{cores}"
            job_log = f"$now'{job_name}'"
            dest_base = f"$SCRATCH/{job_name}_r"
            dest = f"{dest_base}\$LSB_JOBINDEX"
            job = f"mpirun -np {cores} {executable} {n} {time_steps} {ghost_cells} {dest}"
            compute_sub = f"""bsub -n {cores} -W {time_limit} -J '{job_name}[1-{runs}]' -o {job_log} {job}"""
            # https://scicomp.ethz.ch/wiki/Job_arrays
            launch = f"jobid=$({compute_sub} | awk '/is submitted/{{print substr($2, 2, length($2)-2);}}')\n"
            f.write(launch)
            if cleanup:
                cleanup_job = f"rm {dest_base}\*"
                cleanup_sub = f"""bsub -W 1 -J '{job_name}X' -w "numended($jobid,*)" -o {job_log}X {cleanup_job}"""
                cleanup_line = f"""test -n "$jobid" && {cleanup_sub} || echo "Failed to extract job id for {job_name}"\n"""
                f.write(cleanup_line)
        f.write(script_footer)


if __name__ == "__main__":
    generate_jacobi_mpi_sub(**vars(parse_args()))
