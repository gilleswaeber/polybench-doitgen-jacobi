import argparse

executable = './build/jacobi_1D/benchmark/dphpc-jacobi-mpi-benchmark'
final_executable = 'python ./jacobi_1D/jacobi_mpi_collect.py'
script_header = '''#!/bin/bash
# Jacobi 1D submission script
set -euo pipefail

module purge  # clear active modules
module load new gcc/6.3.0 open_mpi/3.0.0 cmake/3.13.5 python/3.7.1
set -x
now=$(date +%m%d%H%M)
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
                        default=10_000_000,
                        type=int)
    parser.add_argument('--time-steps',
                        help='number of iterations',
                        default=1_000,
                        type=int)
    parser.add_argument('--minutes',
                        help='time limit, in minutes',
                        default=5,
                        type=int)
    parser.add_argument('--ghost-cells',
                        help='number of ghost cells (halo size)',
                        default=8,
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
                        help='change director to')

    args = parser.parse_args()
    return args


def generate_jacobi_mpi_sub(*, runs, total_cores, cores_step, output, n, time_steps, ghost_cells, do_cleanup: bool,
                            do_final: bool, send_email: bool, minutes: int, dir: str = None):
    assert runs >= 1
    end_deps = []
    output_dir = "output/$now"
    with output as f:
        f.write(script_header)
        if dir is not None:
            f.write(f"""cd "{dir}"\n""")
        f.write(f"mkdir -p {output_dir}\n")
        f.write(f"chmod u+x mpi_time.sh\n")
        num_cores = [1] + list(range(2, total_cores + 1, cores_step))
        job_base_name = f"jacobi1d_n{n}t{time_steps}g{ghost_cells}"
        f.write(f"bsub -n {total_cores} -o {output_dir}'/{job_base_name}J' -J {job_base_name} -N <<EOF\n")
        f.write(f"set -x\n")
        for cores in reversed(num_cores):
            job_name = f"{job_base_name}c{cores}"
            job_log = f"{output_dir}/{job_name}"
            dest_base = f"$SCRATCH/${{now}}_{job_name}_r"
            for i in range(runs):
                dest = f"{dest_base}{i}"
                time_cmd = f"mpi_time.sh {job_log}r{i}t"
                job = f"mpirun -np {cores} {time_cmd} {executable} {n} {time_steps} {ghost_cells} {dest}\n"
                f.write(job)
            if do_cleanup:
                cleanup_job = f"rm {dest_base}*\n"
                f.write(cleanup_job)

        if do_final:
            final_job = f"{final_executable} {output_dir}\n"
            f.write(final_job)

        f.write("EOF\n")

        f.write(script_footer)


if __name__ == "__main__":
    generate_jacobi_mpi_sub(**vars(parse_args()))
