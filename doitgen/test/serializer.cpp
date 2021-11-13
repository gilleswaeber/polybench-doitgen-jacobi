#include <iostream>
#include <fstream>
#include "serializer.hpp"

void writeFile(uint64_t nr, uint64_t nq, uint64_t np, DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np)) {
	
	std::ofstream wf("doitgen.dat", std::ios::out | std::ios::binary);
	wf.write(reinterpret_cast<char*>(&(A[0][0][0])), nr * nq * np * sizeof(double));
	wf.close();
}

void loadFile(uint64_t nr, uint64_t nq, uint64_t np, DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np)) {
	std::ifstream lf("doitgen.dat", std::ios::in | std::ios::binary);
	lf.read(reinterpret_cast<char*>(&(A[0][0][0])), std::streamsize(nr * nq * np * sizeof(double)));
	lf.close();
}