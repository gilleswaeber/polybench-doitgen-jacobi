bsub -n 48 -W 24:00 -R "select[model==EPYC_7H12]" << EOF
./dphpc-bsub-benchmark 0 sequential batched
./dphpc-bsub-benchmark 1 sequential batched
./dphpc-bsub-benchmark 2 sequential batched
./dphpc-bsub-benchmark 3 sequential batched
./dphpc-bsub-benchmark 4 sequential batched
./dphpc-bsub-benchmark 5 sequential batched
./dphpc-bsub-benchmark 6 sequential batched
./dphpc-bsub-benchmark 7 sequential batched
./dphpc-bsub-benchmark 8 sequential batched
./dphpc-bsub-benchmark 9 sequential batched
./dphpc-bsub-benchmark 0 random batched
./dphpc-bsub-benchmark 1 random batched
./dphpc-bsub-benchmark 2 random batched
./dphpc-bsub-benchmark 3 random batched
./dphpc-bsub-benchmark 4 random batched
./dphpc-bsub-benchmark 5 random batched
./dphpc-bsub-benchmark 6 random batched
./dphpc-bsub-benchmark 7 random batched
./dphpc-bsub-benchmark 8 random batched
./dphpc-bsub-benchmark 9 random batched
EOF
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 0 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 1 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 2 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 3 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 4 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 5 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 6 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 7 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 8 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 9 sequential single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 0 random single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 1 random single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 2 random single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 3 random single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 4 random single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 5 random single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 6 random single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 7 random single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 8 random single
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 9 random single
