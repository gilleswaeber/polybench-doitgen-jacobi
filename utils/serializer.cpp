#include <cstdio>
#include <cassert>
#include "serializer.hpp"

void writeFile(const std::string& name, unsigned long long int size, double* a) {
	FILE* data_file = fopen(name.c_str(), "wb");
	assert(data_file);

	fwrite(a, sizeof(double), size, data_file);
	fclose(data_file);

	/*std::ofstream wf("doitgen.dat", std::ios::out | std::ios::binary);
	wf.write(reinterpret_cast<char*>(&(A[0][0][0])), nr * nq * np * sizeof(double));
	wf.close();*/
}

void loadFile(const std::string& name, unsigned long long int size, double* a) {

	FILE* data_file = fopen(name.c_str(), "rb");
	assert(data_file);
	fread(a, sizeof(double), size, data_file);
	fclose(data_file);
	/*std::ifstream lf("doitgen.dat", std::ios::in | std::ios::binary);
	lf.read(reinterpret_cast<char*>(&(A[0][0][0])), std::streamsize(nr * nq * np * sizeof(double)));
	lf.close();*/
}