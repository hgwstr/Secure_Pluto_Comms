#include "oversample.h"

DVec oversample(const DVec& my_array, int M) {
    int N = static_cast<int>(my_array.size());
    DVec oversampled_array(N * M, 0.0);
    for (int i = 0; i < N; ++i) {
        oversampled_array[i * M] = my_array[i];
    }
    return oversampled_array;
}
