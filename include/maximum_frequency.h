#ifndef MAXIMUM_FREQUENCY_H
#define MAXIMUM_FREQUENCY_H

#include "dsp_utils.h"

// Find frequency of maximum amplitude in FFT of signal
// rx_nth_power: signal raised to Nth power
// fs: sampling frequency
double maximum_frequency(const CVec& rx_nth_power, double fs);

#endif
