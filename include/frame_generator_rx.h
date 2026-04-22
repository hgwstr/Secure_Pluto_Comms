#ifndef FRAME_GENERATOR_RX_H
#define FRAME_GENERATOR_RX_H

#include "dsp_utils.h"
#include <string>

// Extract message from quantized symbols using correlation data
std::string frame_generator_RX(const CVec& quantized_symbols,
                                const CVec& correlation_indices,
                                const CVec& correlation_values,
                                int single_frame_length,
                                const std::string& header,
                                const std::string& modulation_type);

#endif
