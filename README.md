### Dependency

No need to install liblsb.

Otherwise:

```
sudo apt-get install -y openmpi-bin libpapi-dev
sudo apt install r-base-core 
```

Then for the R dependencies run 

```
sudo Rscript ./requirement.R
```

Fedora:
```sh
sudo dnf install openmpi-devel check-devel
sudo dnf install R-core R-ggplot2 R-reshape2 R-data.table
```
and set CMake settings to
```
-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DMPI_HOME=/usr/lib64/openmpi
```

Compile and run on Euler:
```sh
git clean -f -d -x  # clear generated files
lmod2env  # use legacy module system on Euler
module purge  # clear active modules
module load new gcc/6.3.0 open_mpi/3.0.0 cmake/3.13.5 
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release .
make -C ./build/jacobi_1D/test
make -C ./build/jacobi_1D/benchmark
make -C ./build/doitgen/benchmark
CK_DEFAULT_TIMEOUT=120 bsub -I ./build/jacobi_1D/test/dphpc-jacobi-test  # interactive run
bsub -n 48 -W 1:00 mpirun -np 48 ./build/jacobi_1D/benchmark/dphpc-jacobi-mpi-benchmark  # run with 48 proc with 1h of time limit
```

(**don't**: use the new module system on Euler: `set_software_stack.sh new`)