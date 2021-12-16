# Launch doitgen MPI 

example command:

`mpirun -n 4 ./dphpc-doitgen-mpi-test out.txt transpose i7-4770 0 16 128 128`

detailed:

`mpirun -n 4 ./<exec> <outpath> <benchmark_name> <processor_model> <run_index> <nr> <nq> <np>`