#include "maximum_frequency.h"
#include <cmath>
#include <algorithm>

double maximum_frequency(const CVec& rx_nth_power, double fs) {
    int N = static_cast<int>(rx_nth_power.size());

    // Compute FFT
    CVec spectrum = fft(rx_nth_power);

    // fftshift
    CVec shifted = fftshift(spectrum);

    // Compute magnitude (PSD)
    DVec psd(N);
    for (int i = 0; i < N; ++i) {
        psd[i] = std::abs(shifted[i]);
    }

    // Frequency vector: f = arange(-fs/2, fs/2, fs/len(psd))
    DVec f = arange(-fs / 2.0, fs / 2.0, fs / N);

    // Ensure f has same length as psd (may differ by 1 due to floating point)
    if (static_cast<int>(f.size()) > N) {
        f.resize(N);
    }

    // Find index of max magnitude
    int max_idx = 0;
    double max_val = psd[0];
    for (int i = 1; i < N; ++i) {
        if (psd[i] > max_val) {
            max_val = psd[i];
            max_idx = i;
        }
    }

    if (max_idx < static_cast<int>(f.size())) {
        return f[max_idx];
    }
    return 0.0;
}
