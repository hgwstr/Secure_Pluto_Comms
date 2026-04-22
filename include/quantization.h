#ifndef QUANTIZATION_H
#define QUANTIZATION_H

#include "dsp_utils.h"
#include <utility>

// Quantize symbols to nearest constellation points
// Returns: {quantized_symbols, threshold_array}
std::pair<CVec, CVec> quantalph_distance(const CVec& symbol_array,
                                          const DVec& alphabet);

#endif
