bsub -n 48 -W 24:00 -R "select[model==EPYC_7H12]" << EOF
./dphpc-bsub-benchmark 0 sequential
./dphpc-bsub-benchmark 1 sequential
./dphpc-bsub-benchmark 2 sequential
./dphpc-bsub-benchmark 3 sequential
./dphpc-bsub-benchmark 4 sequential
./dphpc-bsub-benchmark 5 sequential
./dphpc-bsub-benchmark 6 sequential
./dphpc-bsub-benchmark 7 sequential
./dphpc-bsub-benchmark 8 sequential
./dphpc-bsub-benchmark 9 sequential
./dphpc-bsub-benchmark 0 random
./dphpc-bsub-benchmark 1 random
./dphpc-bsub-benchmark 2 random
./dphpc-bsub-benchmark 3 random
./dphpc-bsub-benchmark 4 random
./dphpc-bsub-benchmark 5 random
./dphpc-bsub-benchmark 6 random
./dphpc-bsub-benchmark 7 random
./dphpc-bsub-benchmark 8 random
./dphpc-bsub-benchmark 9 random
EOF
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 0 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 1 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 2 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 3 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 4 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 5 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 6 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 7 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 8 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 9 sequential
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 0 random
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 1 random
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 2 random
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 3 random
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 4 random
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 5 random
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 6 random
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 7 random
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 8 random
bsub -n 1 -W 4:00 -R "select[model==EPYC_7H12]" -R "rusage[mem=4096]" ./dphpc-bsub-benchmark 9 random
