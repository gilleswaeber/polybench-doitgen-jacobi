#pragma once

#include <stdint.h>
#include <openmpi/mpi.h>

/**
 * doitgen.h: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
/*
#define CUSTOM_DATASET

#include <../../polybench.hpp>
#include <stdint.h>

#ifndef DOITGEN_H
# define DOITGEN_H


# if !defined(MINI_DATASET) && !defined(SMALL_DATASET) && !defined(LARGE_DATASET) && !defined(EXTRALARGE_DATASET)
#  define STANDARD_DATASET
# endif


# if !defined(NQ) && !defined(NR) && !defined(NP)

#  ifdef MINI_DATASET
#   define NQ 10
#   define NR 10
#   define NP 10
#  endif

#  ifdef SMALL_DATASET
#   define NQ 32
#   define NR 32
#   define NP 32
#  endif

#  ifdef STANDARD_DATASET
#   define NQ 128
#   define NR 128
#   define NP 128
#  endif

#  ifdef LARGE_DATASET
#   define NQ 256
#   define NR 256
#   define NP 256
#  endif

#  ifdef EXTRALARGE_DATASET
#   define NQ 1000
#   define NR 1000
#   define NP 1000
#  endif

#  ifdef CUSTOM_DATASET
#   define NQ 512
#   define NR 512
#   define NP 512
#  endif
# endif

# define _PB_NQ POLYBENCH_LOOP_BOUND(NQ,nq)
# define _PB_NR POLYBENCH_LOOP_BOUND(NR,nr)
# define _PB_NP POLYBENCH_LOOP_BOUND(NP,np)

# ifndef DATA_TYPE
#  define DATA_TYPE double
#  define DATA_PRINTF_MODIFIER "%0.2lf "
# endif

*/

#include <string>

struct problem_size_t {
	uint64_t nr;
	uint64_t nq;
	uint64_t np; 
};

struct problem_instance_t {
	void (*kernel_id)(uint64_t, uint64_t, uint64_t, double*, double*, double*);
	std::string desc;
};

const static uint64_t num_processor = 8;
const static uint64_t PROBLEM_SIZE_N = 5;

const static problem_size_t problem_sizes[] = {
	{10, 10, 10},
	{32, 32, 32},
	{64, 64, 64},
	{256, 256, 256},
	{257, 257, 257}//,
	//{512, 512, 512}
};


#define ARR_2D(ARRAY, Y_DIM, X, Y) (ARRAY[ (X) * (Y_DIM) + (Y) ])

#define C4(X, Y) ARR_2D(c4, np, X, Y)

#define ARR_3D(ARRAY, X_DIM, Y_DIM, Z_DIM, X, Y, Z) \
	(ARRAY[ ((Z_DIM) * (Y_DIM) * (X)) + ((Z_DIM) * (Y)) + (Z) ])

#define A(X, Y, Z) ARR_3D(a, nr, nq, np, X, Y, Z)
#define A_IN(X, Y, Z) ARR_3D(a_in, nr, nq, np, X, Y, Z)
#define A_OUT(X, Y, Z) ARR_3D(a_out, nr, nq, np, X, Y, Z)

#define SUM(X, Y, Z) ARR_3D(sum, nr, nq, np, X, Y, Z)

void init_array(uint64_t nr, uint64_t nq, uint64_t np, double* A, double* C4);
void kernel_doitgen_seq(uint64_t nr, uint64_t nq, uint64_t np, double* a, double* c4, double* sum);
void kernel_doitgen_openmp(uint64_t nr, uint64_t nq, uint64_t np, double* a, double* c4, double* sum);
void kernel_doitgen_experimental(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
);
void kernel_doitgen_transpose(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
);

void kernel_doitgen_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
);

void kernel_doitgen_transpose_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
);

void kernel_doitgen_no_blocking(uint64_t nr, uint64_t nq, uint64_t np,
	double* a_in,
	double* a_out,
	double* c4,
	double* sum
);

void kernel_doitgen_mpi(uint64_t nr, uint64_t nq, uint64_t np,
	double* a,
	double* c4,
	double* sum
);

const static problem_instance_t kernels_to_benchmark[] = {
	{kernel_doitgen_seq, "reference kernel"},
	{kernel_doitgen_openmp, "openMp implementation"}
};

//#endif /* !DOITGEN */
