#include <vector>
#include <omp.h>
#include <iostream>

#include <mpi.h>
#include <liblsb.h>

#include "jacobi1d.hpp"
#include "utils.hpp"


void do_polybench(int n, int tsteps) {
    std::vector<double> A(n);

    init_1d_array(n, A.data());
    flush_cache();

    LSB_Res();
    kernel_jacobi_1d_imper(tsteps, n, A.data());
    LSB_Rec(0);
}

void do_polybench_parallel(int n, int tsteps) {
    std::vector<double> A(n);

    init_1d_array(n, A.data());
    flush_cache_openMP();

    LSB_Res();
    kernel_jacobi_1d_imper_par(tsteps, n, A.data());
    LSB_Rec(0);
}

void do_polybench_swap(int n, int tsteps) {
    std::vector<double> A(n);

    init_1d_array(n, A.data());
    flush_cache_openMP();

    LSB_Res();
    kernel_jacobi_1d_imper_swap(tsteps, n, A.data());
    LSB_Rec(0);
}

typedef void (*benchmark_func)(int, int);

struct Benchmark {
    const char* name;
    benchmark_func func;
};

static const Benchmark benchmarks[] = {
        {"polybench", &do_polybench},
        {"polybench_parallel", &do_polybench_parallel},
        {"polybench_swap", &do_polybench_swap}
};

static const uint64_t benchmarks_size = sizeof(benchmarks) / sizeof(benchmarks[0]);

void help() {
    std::cout << "Usage : ./program benchmark_type nr nq np #threads run_id blocking_window(optional)" << '\n';
    std::cout << "benchmark_type: type of benchmark" << '\n';
    std::cout << "#threads: the number of threads to use" << '\n';
    std::cout << "n tsteps : problem parameters" << '\n';
    std::cout << "run_id : id of the current run" << '\n';
}

int main(int argc, char** argv) {
    if (argc < 6) {
        std::cout << "Too few arguments..." << '\n';
        help();
        return -1;
    }
    if (argc > 6) {
        std::cout << "Too many arguments..." << '\n';
        help();
        return -1;
    }
    const char* benchmark_type = argv[1];
    int n = strtoull(argv[2], nullptr, 0);
    int tsteps = strtoull(argv[3], nullptr, 0);

    int threads = strtoull(argv[4], nullptr, 0);

    uint64_t run_id = strtoull(argv[5], nullptr, 0);

    omp_set_num_threads(threads);

    MPI::Init();

    const std::string benchmark_type_str = benchmark_type;
    const std::string benchmark_name = "jacobi1d-openmp-" + benchmark_type_str + "-" +
                                       std::to_string(n) + "-" + std::to_string(tsteps) + "-" +
                                       std::to_string(threads) + "-" + std::to_string(run_id);
    LSB_Init(benchmark_name.c_str(), 0);

    LSB_Set_Rparam_string("benchmark", benchmark_type);

    LSB_Set_Rparam_long("N", n);
    LSB_Set_Rparam_long("TSTEPS", tsteps);


    LSB_Set_Rparam_long("threads", threads);

    for (uint64_t i = 0; i < benchmarks_size; ++i) {
        if (strcmp(benchmark_type, benchmarks[i].name) == 0) {
            benchmarks[i].func(n, tsteps);
            break;
        }
    }
    LSB_Finalize();
    MPI::Finalize();
    return 0;
}
