#include <doitgen.hpp>
#include <utils.hpp>
#include <chrono>
#include <omp.h>
#include <liblsb.h>


int main() {

    LSB_Init("sequential", 0);

    double* a_in;
	double* a_out;
	double* c4;
	double* sum;


	//init(&a_in, &a_out, &c4, &sum, nr, nq, np);

    LSB_Finalize();

    return 0;
}