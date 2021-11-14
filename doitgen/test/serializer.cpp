#include <cstdio>
#include <cassert>
#include "serializer.hpp"

void writeFile(const char* name, uint64_t nr, uint64_t nq, uint64_t np, DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np)) {
	FILE* data_file = fopen(name, "wb");
	assert(data_file);

	fwrite(&A[0][0][0], sizeof(double), nr * nq * np, data_file);
	fclose(data_file);

	/*std::ofstream wf("doitgen.dat", std::ios::out | std::ios::binary);
	wf.write(reinterpret_cast<char*>(&(A[0][0][0])), nr * nq * np * sizeof(double));
	wf.close();*/
}

void loadFile(const char* name, uint64_t nr, uint64_t nq, uint64_t np, DATA_TYPE POLYBENCH_3D(A, NR, NQ, NP, nr, nq, np)) {

	FILE* data_file = fopen(name, "rb");
	assert(data_file);
	fread(&A[0][0][0], sizeof(double), nr * np * nq, data_file);
	fclose(data_file);
	/*std::ifstream lf("doitgen.dat", std::ios::in | std::ios::binary);
	lf.read(reinterpret_cast<char*>(&(A[0][0][0])), std::streamsize(nr * nq * np * sizeof(double)));
	lf.close();*/
}