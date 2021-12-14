
#include <iostream>

#include "doitgen.hpp"
#include "serializer.hpp"
#include "utils.hpp"

int main(void)
{
    std::cout << "Generating datasets for the sizes" << std::endl;
    
    for (uint64_t i = 0; i < PROBLEM_SIZE_N; i++) {
    
        const problem_size_t& pb_size = problem_sizes[i];

        std::cout << "  " << pb_size.nr << " " << pb_size.nq << " " << pb_size.np << std::endl;

        double* a 	= (double*) allocate_data(pb_size.nr * pb_size.nq * pb_size.np, sizeof(double));
	    double* sum = (double*) allocate_data(pb_size.np, sizeof(double));
	    double* c4 	= (double*) allocate_data(pb_size.np * pb_size.np, sizeof(double));
        
        init_array(pb_size.nr, pb_size.nq, pb_size.np, a, c4);

        kernel_doitgen_seq(pb_size.nr, pb_size.nq, pb_size.np, a, c4, sum);

        std::string name = "doitgen_dataset_" + std::to_string(pb_size.nr) + 
                            "_" + std::to_string(pb_size.nq) + "_" + std::to_string(pb_size.np);
        writeFile(name, pb_size.nr * pb_size.nq * pb_size.np, a);

        cleanup(a);
        cleanup(sum);
        cleanup(c4);

    }

    return 0;
}