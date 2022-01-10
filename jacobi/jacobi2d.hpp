#pragma once
// Author: Gilles Waeber

#include <vector>
#include <ostream>
#include <fstream>
#include <cassert>
#include <cmath>
#include "jacobi1d.hpp"
#include "array2dr.hpp"

void init_2d_array(int n, Array2dR &A);
void init_2d_array(int n, int firstRow, int firstCol, int rows, int cols, Array2dR &A);

void jacobi_2d_reference(int timeSteps, int n, Array2dR &A);
void jacobi_2d_mpi(int timeSteps, int n, MpiParams params);
