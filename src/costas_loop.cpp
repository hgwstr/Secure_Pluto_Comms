#include <cmath>
#include <algorithm>

#include "costas_loop.h"
#include "generate_filter.h"

CostasLoopResult costas_loop_QAM(const DVec& rx, double fs, double mu,
                                  double estimated_frequency,
                                  double theta_init) {
    double f0 = estimated_frequency;
    double Ts = 1.0 / fs;
    int N = static_cast<int>(rx.size());
    double time_total = N * Ts;

    // Time vector
    DVec t = arange(0.0, time_total, Ts);
    int t_len = static_cast<int>(t.size());
    if (t_len > N) t.resize(N);
    t_len = static_cast<int>(t.size());

    // Get filter taps (201-tap Remez LP filter)
    DVec taps = get_costas_lp_taps();
    int fl = static_cast<int>(taps.size()); // 201

    // Reverse taps for dot product (fliplr in Python)
    DVec taps_rev(fl);
    for (int i = 0; i < fl; ++i) {
        taps_rev[i] = taps[fl - 1 - i];
    }

    // Initialize
    DVec theta(t_len, 0.0);
    theta[0] = theta_init;

    DVec z1(fl, 0.0), z2(fl, 0.0), z3(fl, 0.0), z4(fl, 0.0);
    DVec carrier_estimation(N, 0.0);
    CVec complex_exp_estimation(N, Complex(0.0, 0.0));

    for (int k = 0; k < t_len - 1; ++k) {
        double s = 2.0 * rx[k];

        // Shift z arrays left by 1, append new value
        // z1 = append(z1[1:], new_val)
        std::rotate(z1.begin(), z1.begin() + 1, z1.end());
        z1[fl - 1] = s * std::cos(2.0 * M_PI * f0 * t[k] + theta[k]);

        std::rotate(z2.begin(), z2.begin() + 1, z2.end());
        z2[fl - 1] = s * std::cos(2.0 * M_PI * f0 * t[k] + M_PI / 4.0 + theta[k]);

        std::rotate(z3.begin(), z3.begin() + 1, z3.end());
        z3[fl - 1] = s * std::cos(2.0 * M_PI * f0 * t[k] + M_PI / 2.0 + theta[k]);

        std::rotate(z4.begin(), z4.begin() + 1, z4.end());
        z4[fl - 1] = s * std::cos(2.0 * M_PI * f0 * t[k] + 3.0 * M_PI / 4.0 + theta[k]);

        // Apply LP filter (dot product with reversed taps)
        double lpf1 = 0.0, lpf2 = 0.0, lpf3 = 0.0, lpf4 = 0.0;
        for (int i = 0; i < fl; ++i) {
            lpf1 += taps_rev[i] * z1[i];
            lpf2 += taps_rev[i] * z2[i];
            lpf3 += taps_rev[i] * z3[i];
            lpf4 += taps_rev[i] * z4[i];
        }

        // Update theta
        theta[k + 1] = theta[k] + mu * lpf1 * lpf2 * lpf3 * lpf4;

        // Output
        carrier_estimation[k] = std::cos(2.0 * M_PI * f0 * t[k] + theta[k]);
        double phase = 2.0 * M_PI * estimated_frequency * t[k] + theta[k];
        complex_exp_estimation[k] = std::exp(Complex(0.0, -phase));
    }

    return {carrier_estimation, theta, complex_exp_estimation};
}
