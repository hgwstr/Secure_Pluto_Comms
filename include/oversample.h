#ifndef OVERSAMPLE_H
#define OVERSAMPLE_H

#include "dsp_utils.h"

// Zero-insertion oversampling: insert M-1 zeros between each sample
DVec oversample(const DVec& my_array, int M);

#endif
