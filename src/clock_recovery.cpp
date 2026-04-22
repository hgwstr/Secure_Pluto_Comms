#include <cmath>
#include <iostream>

#include "clock_recovery.h"
#include "interpolation_with_sinc.h"

std::pair<DVec, DVec> clock_recovery_OP_max(const DVec& baseband_signal,
                                             double t_now,
                                             int half_number_of_symbols,
                                             int oversampling_factor,
                                             double mu, double delta,
                                             double beta) {
    int m = oversampling_factor;
    int l = half_number_of_symbols;

    if (t_now < m * l / 2.0) {
        std::cout << "tnow is less than min, therefore m*l/2 is assigned" << std::endl;
        t_now = m * l / 2.0;
    }

    int n = static_cast<int>(std::round(
        static_cast<double>(baseband_signal.size()) / oversampling_factor - 2.0 * l));
    DVec xs(n, 0.0);
    DVec tau_save(n, 0.0);
    int i = 0;
    double tau = 0.0;

    int sig_len = static_cast<int>(baseband_signal.size());

    while (t_now < (sig_len - l * m / 4.0)) {
        if (i >= n) break;

        xs[i] = interpolation_with_sinc(baseband_signal, t_now + tau,
                                         l, m, beta);

        double x_deltap = interpolation_with_sinc(baseband_signal,
                                                   t_now + tau + delta,
                                                   l, m, beta);

        double x_deltam = interpolation_with_sinc(baseband_signal,
                                                   t_now + tau - delta,
                                                   l, m, beta);

        double dx = x_deltap - x_deltam;
        tau = tau + mu * dx * xs[i];
        t_now = t_now + m;
        tau_save[i] = tau;
        i = i + 1;
    }

    return {tau_save, xs};
}
