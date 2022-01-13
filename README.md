# Prerequisites
*liblsb* is already included with the sources.

## Euler cluster
Make sure you are using the [new software stack](https://scicomp.ethz.ch/wiki/Setting_permanent_default_for_software_stack_upon_login) on Euler:
`set_software_stack.sh new`

Load necessary modules
```sh
module purge  # clear active modules
module load gcc/9.3.0 openmpi/4.0.2 python/3.6.5 cmake/3.20.3
```

## Debian-like
```
sudo apt update && sudo apt install cmake g++ openmpi-bin libpapi-dev r-base-core 
sudo Rscript ./requirement.R
```

## Fedora
```sh
sudo dnf install cmake g++ openmpi-devel check-devel libasan
sudo dnf install R-core R-ggplot2 R-reshape2 R-data.table R-tidyverse R-quantreg R-docopt R-here
module load mpi
```
# Compile
```sh
cmake -B./build -DCMAKE_BUILD_TYPE=Release .
make -C ./build
```

# Run
Jacobi MPI: use the `jacobi_mpi_sub.py` script to generate submissions.
```sh
python jacobi/jacobi_mpi_sub.py --help
python jacobi/jacobi_mpi_sub.py jacobi1d-mpi-benchmark-lsb --np 1 12-48:12 --ghost-cells 8 --n 10000 | bash
```