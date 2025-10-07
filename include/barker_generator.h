#ifndef BARKER_GENERATOR_H
#define BARKER_GENERATOR_H

#include <complex.h>

double complex* barker_generator(const char* barker_type, const char* modulation_type, int* out_len);

#endif