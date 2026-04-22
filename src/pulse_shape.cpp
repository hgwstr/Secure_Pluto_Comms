#include "pulse_shape.h"
#include <cmath>

DVec srrc(int syms, double beta, int P, double t_off) {
    if (beta == 0.0) {
        beta = 1e-8;
    }

    int length_SRRC = P * syms * 2;
    double start = static_cast<double>(-length_SRRC / 2) + 1e-8 + t_off;
    double stop = static_cast<double>(length_SRRC / 2) + 1e-8 + t_off;

    // Generate k array: from start to stop inclusive, step 1
    DVec k;
    for (double v = start; v <= stop + 0.5; v += 1.0) {
        k.push_back(v);
    }

    DVec s(k.size());
    for (size_t i = 0; i < k.size(); ++i) {
        double kv = k[i];
        double denom = M_PI * (1.0 - 16.0 * std::pow(beta * kv / P, 2.0));
        double numer = (4.0 * beta / std::sqrt(static_cast<double>(P))) *
                       (std::cos((1.0 + beta) * M_PI * kv / P) +
                        std::sin((1.0 - beta) * M_PI * kv / P) /
                            (4.0 * beta * kv / P));
        s[i] = numer / denom;
    }
    return s;
}
