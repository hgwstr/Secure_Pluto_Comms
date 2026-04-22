#ifndef SIGNAL_GENERATOR_TX_H
#define SIGNAL_GENERATOR_TX_H

#include "dsp_utils.h"

// Generate transmit signal from symbol frames
// symbol_frames: complex symbol frames
// buffer_len_TX: TX buffer length (2^18)
// oversampling_rate: samples per symbol (16)
// half_number_of_symbols: half SRRC symbols (6)
// beta: roll-off factor (0.75)
// signal_or_symbols: 1 for convolved signal, 0 for raw symbols
CVec signal_generator(const CVec& symbol_frames,
                      int buffer_len_TX,
                      int oversampling_rate,
                      int half_number_of_symbols,
                      double beta,
                      int signal_or_symbols);

#endif
