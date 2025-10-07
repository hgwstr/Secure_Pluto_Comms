#include "../include/barker_generator.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

double complex* barker_generator(const char* barker_type, const char* modulation_type, int* out_len) {
    
    int len = 0;
    const int* barker_real_ptr = NULL;
    double complex* header = NULL;

    if (strcmp(barker_type, "barker13") == 0) {
        len = 13;
        static const int barker_real[] = {1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1};
        barker_real_ptr = barker_real;
    } else if (strcmp(barker_type, "barker11") == 0) {
        len = 11;
        static const int barker_real[] = {1, 1, 1, -1, -1, -1, 1, -1, -1, 1, -1};
        barker_real_ptr = barker_real;
    } else if (strcmp(barker_type, "barker7") == 0) {
        len = 7;
        static const int barker_real[] = {1, 1, 1, -1, -1, 1, -1};
        barker_real_ptr = barker_real;
    } else {
        printf("Input header\n");
        *out_len = 0;
        return NULL;
    }

    header = (double complex*)malloc(len * sizeof(double complex));
    if (!header) {
        *out_len = 0;
        return NULL;
    }
    
    for (int i = 0; i < len; i++) {
        header[i] = (double)barker_real_ptr[i] + (double)barker_real_ptr[i] * I;
    }

    if (strcmp(modulation_type, "QAM4_2") == 0) {
        for (int i = 0; i < len; i++) {
            header[i] *= 3.0;
        }
    } else if (strcmp(modulation_type, "4PAM") == 0) {
        for (int i = 0; i < len; i++) {
            header[i] = creal(header[i]) * 3.0;
        }
    } else {
        for (int i = 0; i < len; i++) {
            header[i] *= 1.1;
        }
    }

    *out_len = len;
    return header;
}