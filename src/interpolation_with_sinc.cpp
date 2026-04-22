#include "interpolation_with_sinc.h"
#include "pulse_shape.h"
#include <cmath>
#include <algorithm>

double interpolation_with_sinc(const DVec& sampledData, double t,
                                int oneSidedLength, int osFactor,
                                double beta) {
    int nOfLobes = oneSidedLength;
    int P = osFactor;
    int l = P * nOfLobes;

    int tnow = static_cast<int>(std::floor(t));
    double tau = t - std::floor(t);

    // Generate shifted SRRC pulse
    DVec s_tau = srrc(l, beta, 1, tau);

    // Extract window: sampledData[tnow - l : tnow + l + 1]
    int start_idx = tnow - l;
    int end_idx = tnow + l + 1;

    // Bounds check
    int data_len = static_cast<int>(sampledData.size());
    if (start_idx < 0) start_idx = 0;
    if (end_idx > data_len) end_idx = data_len;

    DVec window(sampledData.begin() + start_idx,
                sampledData.begin() + end_idx);

    // Convolve window with s_tau
    DVec x_tau = convolve(window, s_tau, ConvMode::FULL);

    // Extract value at index (2*l) + 2
    int idx = (2 * l) + 2;
    if (idx >= 0 && idx < static_cast<int>(x_tau.size())) {
        return x_tau[idx];
    }
    return 0.0;
}
