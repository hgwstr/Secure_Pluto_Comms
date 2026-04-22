#ifndef SYMBOL_CORRELATION_H
#define SYMBOL_CORRELATION_H

#include "dsp_utils.h"
#include <string>
#include <utility>

// Correlate received symbols with Barker header
// Returns: {correlation_indices (complex), correlation_values (complex)}
std::pair<CVec, CVec> symbol_correlation(const CVec& symbols,
                                          const std::string& modulation_type,
                                          double trigger,
                                          int header_length);

#endif
