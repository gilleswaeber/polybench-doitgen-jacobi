import os
from pathlib import Path
import re
import pprint as pp
from functools import reduce # import needed for python3; builtin in python2
from collections import defaultdict
from typing import overload
from typing_extensions import runtime
import numpy as np

DEBUG = True
white_space_pat = re.compile(r"\s+")

"""returns the path of doitgen benchmark folder"""
def get_benchmark_path():

    path = Path(".")
    root = path.parent.absolute()
    doitgen_benchmark_path = root / "build" / "doitgen" / "benchmark"
    doitgen_benchmark_path_str = str(doitgen_benchmark_path.resolve())

    if (DEBUG):
        print("### benchmark dir ###")
        print(doitgen_benchmark_path_str)

    return doitgen_benchmark_path_str

"""returns the list of benchmark files generated with liblsb"""
def get_benchmark_files():

    path = get_benchmark_path()
    files = os.listdir(path)
    r = re.compile("lsb.*.r*")
    files_filtered = list(filter(r.match, files))

    if (DEBUG):
        print("### all files ###")
        pp.pprint(files)
        print("### benchmark files ###")
        pp.pprint(files_filtered)
    
    return files_filtered


def group_by_benchmark(files):

    bench = {}
    for file in files:
        key = file.split("_")[0].split(".")[1]
        if (key not in bench):
            bench[key] = list()
        bench[key].append(file)

    if (DEBUG):
        print("### Grouped benchmarks ###")
        pp.pprint(bench)

    return bench

def group_by_core(files):
    bench_per_cores = {}
    for file in files:
        key = file.split("_")[1][:1] # extract the number of cores
        if (key not in bench_per_cores):
            bench_per_cores[key] = list()
        bench_per_cores[key].append(file)
    if (DEBUG):
        print("### grouped by core for some benchmark ###")
        pp.pprint(bench_per_cores)

    return bench_per_cores

"""<name>_i.r0 with <name>_i.r0-1 and <name>_i.r0-2 ..."""
def group_processes_by_run(files):
    bench_per_run = {}
    for file in files:
        key = file.split(".")[2][:2] # extract r0 or r1 etc...
        if (key not in bench_per_run):
            bench_per_run[key] = list()
        bench_per_run[key].append(file) 
    return bench_per_run

"""parse these damned files r, returns the run time, and the stats for each phase (0, 1, ..)"""
def parse_result_file(file_path):

    f = open(file_path, "r")

    lines = f.readlines()
    overhead_line = lines[-1]
    overhead_line = overhead_line.split(" ")
    
    #runtime = float(overhead_line[2])
    overhead = float(overhead_line[5])

    lines = list(filter(lambda x: "#" not in x, lines))
    lines = lines[1:] # remove the header
    lines = [white_space_pat.sub(" ", f.strip()).strip() for f in lines]
    lines = [f.split(" ") for f in lines]
    lines = [(int(f[0]), float(f[1]) / 1000.0, int(f[2])) for f in lines]

    data = {}
    for id, t, o in lines:
        if (id not in data):
            data[id] = list()
        else:
            data[id].append(t)

    f.close()
    return overhead, data

def summarize_single_run(run_dict):
    # want to get (total, '0' : [min, max, avg, median])
    None

def summarize_process_runs(process_data):

    num_runs = len(process_data)
    max_overhead = -1

    for run in process_data:
        overhead = run[0]
        measures = run[1]        
        max_overhead = overhead if (overhead > max_overhead) else max_overhead



    num_measures = len(process_data[0][1])

def create_benchmarks_results(benchmarks_grouped):

    benchmarks = []
    for some_bench in benchmarks_grouped:
        
        new_benchmark = benchmark_result(some_bench) # create empty benchmark
        files_grouped_by_proc = group_by_core(benchmarks_grouped[some_bench])

        process_results = []

        for process_num in files_grouped_by_proc:

            #get the files grouped by thier total num of processes
            files = files_grouped_by_proc[process_num]

            #get the files grouped for each process run
            porcess_to_run_files = group_processes_by_run(files)

            path = get_benchmark_path()

            for ri in porcess_to_run_files:
                # we get a list for proc i of all the runs data
                parsed_runs = [parse_result_file(path + "/" + f) for f in porcess_to_run_files[ri]]
                summarize_process_runs(parsed_runs)
                None
            
            #pp.pprint(parsed_files)

            group_processes_by_run(files) 

            a = parsed_files[0]
            t = a[0]
            sum = 0
            m = a[2]
            for key in m:
                l = np.array(m[key])
                sum += l.sum()
            None


class benchmark_result():
    def __init__(self, name):
        self.name = name
        self.process_indices = None
        self.time_per_process = None
        self.CI = None
        self.process_results = None

class process_result():
    def __init__(self):
        runs = None
        results = None
        overheads = None


def main():

    files = get_benchmark_files()
    benchmarks_grouped = group_by_benchmark(files)

    create_benchmarks_results(benchmarks_grouped)

    #for key in benchmarks_grouped:
    #    group_by_core(benchmarks_grouped[key])

    #for bench in benchmarks_grouped:


if __name__ == "__main__":
    main()