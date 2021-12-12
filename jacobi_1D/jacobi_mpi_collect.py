import argparse
import re
from pandas import DataFrame
import pandas as pd
from pathlib import Path

NAME_PATTERN = re.compile(
    r"^jacobi1d_n(?P<n>\d+)t(?P<time_steps>\d+)g(?P<ghost_cells>\d+)c(?P<cores>\d+)r(?P<run>\d+)t(?P<rank>\d+)"
    r"(v(?P<alternative>.+))?$")
NAME_GROUPS = ('n', 'time_steps', 'ghost_cells', 'cores', 'run', 'rank', 'alternative')
GNU_TIME_PATTERN = re.compile(r"""^
    (?P<user_time>[\d.]+)user\s
    (?P<system_time>[\d.]+)system\s
    (((?P<elapsed_h>[\d.]+):)?(?P<elapsed_m>[\d.]+):)?(?P<elapsed_s>[\d.]+)elapsed\s
    (?P<percent_cpu>[\d]+)%CPU\s
    \((?P<text_kb>[\d]+)avgtext\+(?P<data_kb>[\d]+)avgdata\s(?P<mem_kb>[\d]+)maxresident\)k\s+
    (?P<fs_in>[\d]+)inputs\+(?P<fs_out>[\d]+)outputs\s
    \((?P<faults_maj>[\d]+)major\+(?P<faults_min>[\d]+)minor\)pagefaults\s
    (?P<swaps>[\d]+)swaps\s+$""", re.VERBOSE)
GNU_TIME_GROUPS = (
    'user_time', 'system_time', 'elapsed_h', 'elapsed_m', 'elapsed_s', 'percent_cpu', 'text_kb', 'data_kb', 'mem_kb',
    'fs_in', 'fs_out', 'faults_maj', 'faults_min', 'swaps')
NUMERIC_GROUPS = ['n', 'time_steps', 'ghost_cells', 'cores', 'run', 'rank', 'user_time', 'system_time', 'percent_cpu',
                  'text_kb', 'data_kb', 'mem_kb', 'fs_in', 'fs_out', 'faults_maj', 'faults_min', 'swaps']
SORT_GROUPS = ['alternative', 'n', 'time_steps', 'ghost_cells', 'cores', 'run', 'rank']
ALL_GROUPS = NAME_GROUPS + GNU_TIME_GROUPS


def parse_args():
    parser = argparse.ArgumentParser(description='Generate Euler script for jacobi-1d')
    parser.add_argument('folder',
                        type=Path,
                        help='folder containing the files')
    args = parser.parse_args()
    return args


def collect_results(folder: Path, write_report=True):
    records = []
    for f in folder.iterdir():
        m = NAME_PATTERN.match(f.stem)
        if f.is_file() and m:
            n = GNU_TIME_PATTERN.match(f.read_text(encoding='utf-8'))
            if n:
                name_info = {g: m.group(g) for g in NAME_GROUPS}
                time_info = {g: n.group(g) for g in GNU_TIME_GROUPS}
                records.append({**name_info, **time_info})
            else:
                print(f'Failed to extract time information from {f}')
    df = DataFrame.from_records(records, columns=ALL_GROUPS)
    df['elapsed_time'] = pd.to_numeric(df.elapsed_h.fillna(0)) * 3600 + pd.to_numeric(
        df.elapsed_m.fillna(0)) * 60 + pd.to_numeric(df.elapsed_s)
    df.drop(columns=['elapsed_h', 'elapsed_m', 'elapsed_s'], inplace=True)
    df[NUMERIC_GROUPS] = df[NUMERIC_GROUPS].apply(pd.to_numeric)
    df.sort_values(SORT_GROUPS, inplace=True)
    df.reset_index(inplace=True)
    df['alternative'].fillna('base', inplace=True)

    # take the maximum time for each run
    by_run = df.groupby(['alternative', 'n', 'time_steps', 'ghost_cells', 'cores', 'run'])['elapsed_time'] \
        .max().reset_index()
    # group the results for each test
    by_test = by_run.groupby(['alternative', 'n', 'time_steps', 'ghost_cells', 'cores'])['elapsed_time'] \
        .aggregate(["min", "max", "mean", "std"]).reset_index()

    if write_report:
        report_file = folder / 'report_full.csv'
        summary_file = folder / 'report_summary.csv'
        df.to_csv(report_file)
        print('Complete report written in ', report_file)
        by_test.to_csv(summary_file)
        print('Summary written in ', summary_file, '\n')

        print(by_test.to_string())

    return df, by_test


if __name__ == '__main__':
    collect_results(**vars(parse_args()))
