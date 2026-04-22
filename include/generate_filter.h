#ifndef GENERATE_FILTER_H
#define GENERATE_FILTER_H

#include "dsp_utils.h"
#include <string>

// Generate filter taps
// For LP filter used in Costas loop: uses hardcoded 201-tap Remez coefficients
// fs: sampling frequency
// filter_type: "lp" or "bp"
// center_freq, bandwidth, transition_width: filter parameters
// number_of_taps: number of filter taps
DVec generate_filter(double fs, const std::string& filter_type,
                     double center_freq, double bandwidth,
                     double transition_width, int number_of_taps);

// Get the hardcoded 201-tap Costas loop LP filter
DVec get_costas_lp_taps();

#endif
