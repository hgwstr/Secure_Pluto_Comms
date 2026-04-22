#ifndef INTERPOLATION_WITH_SINC_H
#define INTERPOLATION_WITH_SINC_H

#include "dsp_utils.h"

// Sinc-based interpolation using SRRC pulse
// sampledData: input samples
// t: time at which to interpolate
// oneSidedLength: half number of symbols for SRRC
// osFactor: oversampling factor
// beta: roll-off factor
double interpolation_with_sinc(const DVec& sampledData, double t,
                                int oneSidedLength, int osFactor,
                                double beta = 0.0);

#endif
