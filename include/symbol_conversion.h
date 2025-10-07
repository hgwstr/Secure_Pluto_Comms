#ifndef SYMBOL_CONVERSION_H
#define SYMBOL_CONVERSION_H

#include <complex.h>

char* pam_to_letters(const int* symbols_PAM, int N);

int* letters_to_pam(const char* text, int* out_len);

int* qam_to_pam(const double complex* symbols_QAM, int N);

double complex* pam_to_qam(const int* symbols_PAM, int N);

int* qam4_2_to_pam(const double complex* symbols_QAM4_2, int N);

double complex* pam_to_qam4_2(const int* symbols_PAM, int N);

double* pam(int len, int M, double var);

#endif