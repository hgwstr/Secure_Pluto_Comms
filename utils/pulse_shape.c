#include "../include/pulse_shape.h"
#include <math.h>
#include <stdlib.h>

double* srrc(int syms, double beta, int P, double t_off, int* out_len) {
    int length_SRRC = P * syms * 2;
    double start = (double)(-length_SRRC / 2) + 1e-8 + t_off;
    double stop = (double)(length_SRRC / 2) + 1e-8 + t_off;
    
    int len = (int)(stop - start) + 1;
    double* s = (double*)malloc(len * sizeof(double));
    if (s == NULL) {
        *out_len = 0;
        return NULL;
    }
    
    if (beta == 0.0) {
        beta = 1e-8;
    }

    int i = 0;
    for (double k = start; i < len; k += 1.0, i++) {
        double denom = (M_PI * (1 - 16 * pow((beta * k / P), 2)));
        
        double sin_part;
        if (k == 0.0) {
           sin_part = (1 - beta) * M_PI / (4 * beta);
        } else {
           sin_part = sin((1 - beta) * M_PI * k / P) / (4 * beta * k / P);
        }

        s[i] = (4 * beta / sqrt(P)) * (cos((1 + beta) * M_PI * k / P) + sin_part) / denom;
    }
    
    *out_len = len;
    return s;
}