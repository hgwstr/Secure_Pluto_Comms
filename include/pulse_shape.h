#ifndef PULSE_SHAPE_H
#define PULSE_SHAPE_H

#include "dsp_utils.h"

// Generate Square-Root Raised Cosine (SRRC) pulse
// syms: half of total number of symbols
// beta: roll-off factor
// P: oversampling factor
// t_off: time offset (default 0)
DVec srrc(int syms, double beta, int P, double t_off = 0.0);

#endif
