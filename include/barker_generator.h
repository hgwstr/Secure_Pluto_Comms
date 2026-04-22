#ifndef BARKER_GENERATOR_H
#define BARKER_GENERATOR_H

#include "dsp_utils.h"
#include <string>

// Generate Barker code header based on type and modulation
// barker_type: "barker13", "barker11", or "barker7"
// modulation_type: "QAM4_2", "4PAM", or other
CVec barker_generator(const std::string& barker_type, const std::string& modulation_type);

#endif
