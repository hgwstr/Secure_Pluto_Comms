#ifndef COSTAS_LOOP_H
#define COSTAS_LOOP_H

#include "dsp_utils.h"

struct CostasLoopResult {
    DVec carrier_estimation;
    DVec theta;
    CVec complex_exp_estimation;
};

// QAM Costas loop for carrier recovery
CostasLoopResult costas_loop_QAM(const DVec& rx, double fs, double mu,
                                  double estimated_frequency,
                                  double theta_init);

#endif
