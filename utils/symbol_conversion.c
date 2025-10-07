#include "../include/symbol_conversion.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

char* pam_to_letters(const int* symbols_PAM, int N) {
    int off = N % 4;
    if (off != 0) {
        N = N - off;
    }

    char* my_4base = (char*)malloc(5 * sizeof(char));
    my_4base[0] = '\0';
    
    char* converted_string = (char*)malloc((N / 4 + 1) * sizeof(char));
    converted_string[0] = '\0';

    int base_idx = 0;
    int converted_idx = 0;

    for (int i = 0; i < N; i++) {
        my_4base[base_idx++] = (char)((symbols_PAM[i] + 99) / 2);
        
        if (base_idx == 4) {
            my_4base[4] = '\0';
            converted_string[converted_idx++] = (char)strtol(my_4base, NULL, 4);
            base_idx = 0;
        }
    }
    
    converted_string[converted_idx] = '\0';
    free(my_4base);

    return converted_string;
}

static void to_base4(int value, char* buffer) {
    if (value == 0) {
        strcpy(buffer, "0");
        return;
    }
    
    char temp[33];
    int i = 0;
    while (value > 0) {
        temp[i++] = (value % 4) + '0';
        value /= 4;
    }
    
    int k = 0;
    for (int j = i - 1; j >= 0; j--) {
        buffer[k++] = temp[j];
    }
    buffer[k] = '\0';
}

int* letters_to_pam(const char* text, int* out_len) {
    int text_len = strlen(text);
    char* msg = (char*)malloc((text_len * 4 + 1) * sizeof(char));
    msg[0] = '\0';
    char letter_buf[33];
    char temp_buf[5];

    for (int i = 0; i < text_len; i++) {
        to_base4((int)text[i], letter_buf);
        if (strlen(letter_buf) == 4) {
             strcat(msg, letter_buf);
        } else {
            temp_buf[0] = '0';
            temp_buf[1] = '\0';
            strcat(temp_buf, letter_buf);
            strcat(msg, temp_buf);
        }
    }

    int msg_len = strlen(msg);
    int* symbols_PAM = (int*)malloc(msg_len * sizeof(int));

    for (int i = 0; i < msg_len; i++) {
        symbols_PAM[i] = 2 * (int)msg[i] - 99;
    }
    
    free(msg);
    *out_len = msg_len;
    return symbols_PAM;
}


int* qam_to_pam(const double complex* symbols_QAM, int N) {
    int* symbols_PAM = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        if (creal(symbols_QAM[i]) == -1 && cimag(symbols_QAM[i]) == -1) {
            symbols_PAM[i] = -3;
        } else if (creal(symbols_QAM[i]) == -1 && cimag(symbols_QAM[i]) == 1) {
            symbols_PAM[i] = -1;
        } else if (creal(symbols_QAM[i]) == 1 && cimag(symbols_QAM[i]) == -1) {
            symbols_PAM[i] = 1;
        } else {
            symbols_PAM[i] = 3;
        }
    }
    return symbols_PAM;
}

double complex* pam_to_qam(const int* symbols_PAM, int N) {
    double complex* symbols_QAM = (double complex*)malloc(N * sizeof(double complex));
    for (int i = 0; i < N; i++) {
        if (symbols_PAM[i] == -3) {
            symbols_QAM[i] = -1.0 - 1.0 * I;
        } else if (symbols_PAM[i] == -1) {
            symbols_QAM[i] = -1.0 + 1.0 * I;
        } else if (symbols_PAM[i] == 1) {
            symbols_QAM[i] = 1.0 - 1.0 * I;
        } else {
            symbols_QAM[i] = 1.0 + 1.0 * I;
        }
    }
    return symbols_QAM;
}

int* qam4_2_to_pam(const double complex* symbols_QAM4_2, int N) {
    int* symbols_PAM = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        if (creal(symbols_QAM4_2[i]) == -3 && cimag(symbols_QAM4_2[i]) == -3) {
            symbols_PAM[i] = -3;
        } else if (creal(symbols_QAM4_2[i]) == -1 && cimag(symbols_QAM4_2[i]) == -1) {
            symbols_PAM[i] = -1;
        } else if (creal(symbols_QAM4_2[i]) == 1 && cimag(symbols_QAM4_2[i]) == 1) {
            symbols_PAM[i] = 1;
        } else {
            symbols_PAM[i] = 3;
        }
    }
    return symbols_PAM;
}

double complex* pam_to_qam4_2(const int* symbols_PAM, int N) {
    double complex* symbols_QAM4_2 = (double complex*)malloc(N * sizeof(double complex));
    for (int i = 0; i < N; i++) {
        if (symbols_PAM[i] == -3) {
            symbols_QAM4_2[i] = -3.0 - 3.0 * I;
        } else if (symbols_PAM[i] == -1) {
            symbols_QAM4_2[i] = -1.0 - 1.0 * I;
        } else if (symbols_PAM[i] == 1) {
            symbols_QAM4_2[i] = 1.0 + 1.0 * I;
        } else {
            symbols_QAM4_2[i] = 3.0 + 3.0 * I;
        }
    }
    return symbols_QAM4_2;
}

double* pam(int len, int M, double var) {
    double* sequence = (double*)malloc(len * sizeof(double));
    double scale = sqrt(3 * var / (M * M - 1));

    for (int i = 0; i < len; i++) {
        double rand_val = (double)rand() / (double)RAND_MAX;
        sequence[i] = (2 * floor(M * rand_val) - M + 1) * scale;
    }
    return sequence;
}