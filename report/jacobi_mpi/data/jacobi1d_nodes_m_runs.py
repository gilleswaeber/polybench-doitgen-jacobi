import pandas as pd
from pandas import DataFrame


def run():
    src = [
        ('0112164900_no1M_lsb_runs.csv', 1),
        ('0113033458_no2M_lsb_runs.csv', 2),
        ('0112164922_no4M_lsb_runs.csv', 4),
        ('0112164932_no6M_lsb_runs.csv', 6),
        ('0113034907_no8M_lsb_runs.csv', 8),
        ('0112165024_no16M_lsb_runs.csv', 16),
        ('0112165034_no24M_lsb_runs.csv', 24),
        ('0113122558_no48M_lsb_runs.csv', 48),
    ]

    all_df = []
    for csv, nodes in src:
        node_df: DataFrame = pd.read_csv(csv)
        node_df.insert(4, 'nodes', nodes)
        all_df.append(node_df)

    all_df = pd.concat(all_df)
    all_df = all_df[all_df['cores'] > 1]
    all_df.sort_values(['alternative', 'n', 'time_steps', 'ghost_cells', 'cores', 'nodes', 'run'], inplace=True)
    all_df.to_csv('jacobi1d_nodes_m_runs.csv', index=False)


if __name__ == '__main__':
    run()
