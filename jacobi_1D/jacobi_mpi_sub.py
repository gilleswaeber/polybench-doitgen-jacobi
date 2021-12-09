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

#LSB_OUTPUT_FORMAT=accumulated
#export LSB_OUTPUT_FORMAT
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
                        default=1_000_000,
                        type=int)
    parser.add_argument('--time-steps',
                        help='number of iterations',
                        default=10_000,
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


def generate_jacobi_mpi_sub(*, runs, total_cores, output, n, time_steps, ghost_cells, do_cleanup: bool, do_final: bool,
                            send_email: bool, minutes: int, dir: str = None):
    assert runs >= 1
    end_deps = []
    output_dir = "output/$now"
    with output as f:
        f.write(script_header)
        if dir is not None:
            f.write(f"""cd "{dir}"\n""")
        f.write(f"mkdir -p {output_dir}\n")
        num_cores = [1] + list(range(2, total_cores + 1, 2))
        job_base_name = f"jacobi1d_n{n}t{time_steps}g{ghost_cells}"
        for cores in reversed(num_cores):
            job_name = f"{job_base_name}c{cores}"
            f.write(f"job={job_name}\n")
            job_log = f"{output_dir}/$job"
            dest_base = f"$SCRATCH/$job'_r'"
            dest = f"{dest_base}\$LSB_JOBINDEX"
            time_cmd = f"mpi_time.sh {job_log}'r'\${{LSB_JOBINDEX}}t"
            # job = f"LSB_OUTFILE=output/$now$job'_'\$LSB_JOBINDEX mpirun -np {cores} {time_cmd} {executable} {n} {time_steps} {ghost_cells} {dest}"
            job = f"mpirun -np {cores} {time_cmd} {executable} {n} {time_steps} {ghost_cells} {dest}"
            compute_sub = f"""bsub -n {cores} -W {minutes} -J $job'[1-{runs}]' -o {job_log} {job}"""
            # https://scicomp.ethz.ch/wiki/Job_arrays
            job_line = f"jobid=$({compute_sub} | awk '/is submitted/{{print substr($2, 2, length($2)-2);}}')\n"
            f.write(job_line)
            if do_cleanup:
                cleanup_job = f"rm {dest_base}\*"
                cleanup_name = f"{job_name}X"
                cleanup_sub = f"""bsub -W 1 -J {cleanup_name} -w "numended($jobid,*)" -o {job_log}'X' {cleanup_job}"""
                cleanup_line = f"""test -n "$jobid" && {cleanup_sub} || echo "Failed to extract job id for $job"\n"""
                end_deps.append(f"ended({cleanup_name})")
                f.write(cleanup_line)

        if do_cleanup and do_final:
            final_job = f"{final_executable} {output_dir}"
            final_cond = ' && '.join(end_deps)
            final_name = f"{job_base_name}F"
            final_log = f"output/$now/{job_base_name}F"
            final_sub_flags = f"""-W 5 -J {final_name} -w "{final_cond}" -o {final_log}"""
            if send_email:
                final_sub_flags += " -N"
            final_line = f"bsub {final_sub_flags} {final_job}\n"
            f.write(final_line)

        f.write(script_footer)


if __name__ == "__main__":
    generate_jacobi_mpi_sub(**vars(parse_args()))
