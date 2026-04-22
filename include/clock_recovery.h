#ifndef CLOCK_RECOVERY_H
#define CLOCK_RECOVERY_H

#include "dsp_utils.h"
#include <utility>

// Clock recovery using output power maximization
// Returns: {tau_save, xs (recovered samples)}
std::pair<DVec, DVec> clock_recovery_OP_max(const DVec& baseband_signal,
                                             double t_now,
                                             int half_number_of_symbols,
                                             int oversampling_factor,
                                             double mu, double delta,
                                             double beta);

#endif
