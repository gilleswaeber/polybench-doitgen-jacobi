import argparse
import re
from typing import Tuple

from pandas import DataFrame
import pandas as pd
from pathlib import Path

GROUP_BY_TEST = ['alternative', 'n', 'time_steps', 'ghost_cells', 'cores']
GROUP_BY_RUN = GROUP_BY_TEST + ['run']

TIME_NAME_PATTERN = re.compile(
    r"^jacobi1d_n(?P<n>\d+)t(?P<time_steps>\d+)g(?P<ghost_cells>\d+)c(?P<cores>\d+)(v(?P<alternative>.+))?"
    r"r(?P<run>\d+)t(?P<rank>\d+)"
    r"$")
TIME_NAME_GROUPS = ('n', 'time_steps', 'ghost_cells', 'cores', 'run', 'rank', 'alternative')
TIME_TEXT_PATTERN = re.compile(r"""^
    (?P<user_time>[\d.]+)user\s
    (?P<system_time>[\d.]+)system\s
    (((?P<elapsed_h>[\d.]+):)?(?P<elapsed_m>[\d.]+):)?(?P<elapsed_s>[\d.]+)elapsed\s
    (?P<percent_cpu>[\d]+)%CPU\s
    \((?P<text_kb>[\d]+)avgtext\+(?P<data_kb>[\d]+)avgdata\s(?P<mem_kb>[\d]+)maxresident\)k\s+
    (?P<fs_in>[\d]+)inputs\+(?P<fs_out>[\d]+)outputs\s
    \((?P<faults_maj>[\d]+)major\+(?P<faults_min>[\d]+)minor\)pagefaults\s
    (?P<swaps>[\d]+)swaps\s+$""", re.VERBOSE)
TIME_TEXT_GROUPS = (
    'user_time', 'system_time', 'elapsed_h', 'elapsed_m', 'elapsed_s', 'percent_cpu', 'text_kb', 'data_kb', 'mem_kb',
    'fs_in', 'fs_out', 'faults_maj', 'faults_min', 'swaps')
LSB_NAME_PATTERN = re.compile(
    r"^lsb\.jacobi1d_n(?P<n>\d+)t(?P<time_steps>\d+)g(?P<ghost_cells>\d+)c(?P<cores>\d+)(v(?P<alternative>.+))?"
    r"r(?P<run>\d+)t\.r(?P<rank>\d+)$")
LSB_NAME_GROUPS = ('n', 'time_steps', 'ghost_cells', 'cores', 'run', 'rank', 'alternative')
LSB_IDS = {
    0: 'init_time',
    1: 'compute_time',
    2: 'sync_time',
    3: 'write_time'
}

NUMERIC_GROUPS = ['n', 'time_steps', 'ghost_cells', 'cores', 'run', 'rank', 'user_time', 'system_time', 'percent_cpu',
                  'text_kb', 'data_kb', 'mem_kb', 'fs_in', 'fs_out', 'faults_maj', 'faults_min', 'swaps']
SORT_GROUPS = ['alternative', 'n', 'time_steps', 'ghost_cells', 'cores', 'run', 'rank']
ALL_GROUPS = TIME_NAME_GROUPS + TIME_TEXT_GROUPS


def parse_args():
    parser = argparse.ArgumentParser(description='Generate Euler script for jacobi-1d')
    parser.add_argument('folder',
                        type=Path,
                        help='folder containing the files')
    args = parser.parse_args()
    return args


def collect_results(folder: Path, write_report=True):
    records = []
    lsb_records_full = []
    lsb_by_rank = []
    for f in folder.iterdir():
        if not f.is_file():
            continue

        time_name = TIME_NAME_PATTERN.match(f.name)
        lsb_name = LSB_NAME_PATTERN.match(f.name)
        if time_name:
            n = TIME_TEXT_PATTERN.match(f.read_text(encoding='utf-8'))
            if n:
                name_info = {g: time_name.group(g) for g in TIME_NAME_GROUPS}
                time_info = {g: n.group(g) for g in TIME_TEXT_GROUPS}
                records.append({**name_info, **time_info})
            else:
                print(f'Failed to extract time information from {f}')
        elif lsb_name:
            lsb, node = read_lsb(f)
            name_info = {g: (pd.to_numeric(lsb_name.group(g)) if g in NUMERIC_GROUPS else lsb_name.group(g))
                         for g in LSB_NAME_GROUPS}
            for g in reversed(LSB_NAME_GROUPS):
                lsb.insert(0, g, name_info[g])
            lsb_records_full.append(lsb)
            grp = lsb.groupby('id')
            lsb_by_rank.append({
                **name_info,
                "total_time": lsb['time'].sum(),
                "init_time": grp.get_group(0)['time'].sum(),
                "compute_time": grp.get_group(1)['time'].sum(),
                "sync_time": grp.get_group(2)['time'].sum() if 2 in grp.groups else 0,
                "write_time": grp.get_group(3)['time'].sum(),
                "node": node
            })
    df = DataFrame.from_records(records, columns=ALL_GROUPS)
    df['elapsed_time'] = pd.to_numeric(df.elapsed_h.fillna(0)) * 3600 + pd.to_numeric(
        df.elapsed_m.fillna(0)) * 60 + pd.to_numeric(df.elapsed_s)
    df.drop(columns=['elapsed_h', 'elapsed_m', 'elapsed_s'], inplace=True)
    df[NUMERIC_GROUPS] = df[NUMERIC_GROUPS].apply(pd.to_numeric)
    df.sort_values(SORT_GROUPS, inplace=True)
    df.reset_index(inplace=True)
    df['alternative'].fillna('base', inplace=True)

    # take the maximum time for each run
    by_run = df.groupby(GROUP_BY_RUN)['elapsed_time'] \
        .max().reset_index()
    # group the results for each test
    by_test = by_run.groupby(GROUP_BY_TEST)['elapsed_time'] \
        .aggregate(["min", "max", "mean", "std"]).reset_index()

    if len(lsb_records_full):
        lsb_records_full = pd.concat(lsb_records_full, ignore_index=True)
        lsb_by_rank = DataFrame.from_records(lsb_by_rank)
        lsb_by_rank['kernel_time'] = lsb_by_rank['compute_time'] + lsb_by_rank['sync_time']
        lsb_by_rank['effective_time'] = lsb_by_rank['kernel_time'] + lsb_by_rank['write_time']
        lsb_by_run = lsb_by_rank.groupby(GROUP_BY_RUN)\
            .agg(kernel_time=('kernel_time', 'max'), effective_time=('effective_time', 'max')).reset_index()

    if write_report:
        report_file = folder / 'report_time_full.csv'
        summary_file = folder / 'report_time_summary.csv'
        lsb_file = folder / 'report_lsb_full.csv'
        lsb_rank_file = folder / 'report_lsb_rank.csv'
        lsb_runs_file = folder / 'report_lsb_runs.csv'
        df.to_csv(report_file, index=False)
        print('Complete report written in ', report_file)
        by_test.to_csv(summary_file, index=False)
        print('Summary written in ', summary_file, '\n')
        if len(lsb_records_full):
            lsb_records_full.to_csv(lsb_file, index=False)
            print('LSB full database written in', lsb_file)
            lsb_by_rank.to_csv(lsb_rank_file, index=False)
            print('LSB summary by rank written in', lsb_rank_file)
            lsb_by_run.to_csv(lsb_runs_file, index=False)
            print('LSB summary by run written in', lsb_runs_file)

        print(by_test.to_string())

    return df, by_test


def read_lsb(path: Path) -> Tuple[DataFrame, str]:
    data = pd.read_csv(path, sep=r'\s+', comment='#')
    to_remove = [c for c in data.columns if re.match(r'^P\d+$', c)]
    data.drop(columns=to_remove, inplace=True)
    with path.open('rt', encoding='utf-8') as f:
        _sys_line = f.readline()
        node = f.readline().split(': ', 1)[1]
    return data, node


if __name__ == '__main__':
    collect_results(**vars(parse_args()))
