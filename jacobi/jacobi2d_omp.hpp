#pragma once

void init_array(int n, std::vector<double> *A);
void kernel_jacobi_2d_imper(int tsteps, int n, std::vector<double> *A);
void kernel_jacobi_2d_imper_par(int tsteps, int n, std::vector<double> *A);
void kernel_jacobi_2d_imper_swap(int tsteps, int n, std::vector<double> *A);