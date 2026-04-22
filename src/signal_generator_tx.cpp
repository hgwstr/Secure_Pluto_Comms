#include "signal_generator_tx.h"
#include "pulse_shape.h"
#include "oversample.h"
#include <cmath>

CVec signal_generator(const CVec& symbol_frames,
                      int buffer_len_TX,
                      int oversampling_rate,
                      int half_number_of_symbols,
                      double beta,
                      int signal_or_symbols) {
    int P = oversampling_rate;

    // Separate real and imaginary parts
    int num_symbols = static_cast<int>(symbol_frames.size());
    DVec real_parts(num_symbols), imag_parts(num_symbols);
    for (int i = 0; i < num_symbols; ++i) {
        real_parts[i] = symbol_frames[i].real();
        imag_parts[i] = symbol_frames[i].imag();
    }

    // Oversample
    DVec oversampled_real = oversample(real_parts, P);
    DVec oversampled_imag = oversample(imag_parts, P);

    int block_len = static_cast<int>(oversampled_real.size());

    // Tile/repeat to fill buffer_len_TX
    int full_tiles = buffer_len_TX / block_len;
    int partial = buffer_len_TX % block_len;

    DVec appended_real;
    DVec appended_imag;
    appended_real.reserve(buffer_len_TX);
    appended_imag.reserve(buffer_len_TX);

    for (int t = 0; t < full_tiles; ++t) {
        appended_real.insert(appended_real.end(), oversampled_real.begin(), oversampled_real.end());
        appended_imag.insert(appended_imag.end(), oversampled_imag.begin(), oversampled_imag.end());
    }
    appended_real.insert(appended_real.end(), oversampled_real.begin(), oversampled_real.begin() + partial);
    appended_imag.insert(appended_imag.end(), oversampled_imag.begin(), oversampled_imag.begin() + partial);

    CVec my_signal;

    if (signal_or_symbols) {
        // Generate SRRC pulse
        DVec pulse = srrc(half_number_of_symbols, beta, P);

        // Convolve
        DVec signal_real = convolve(appended_real, pulse, ConvMode::FULL);
        DVec signal_imag = convolve(appended_imag, pulse, ConvMode::FULL);

        // Combine into complex signal
        int sig_len = static_cast<int>(signal_real.size());
        my_signal.resize(sig_len);
        for (int i = 0; i < sig_len; ++i) {
            my_signal[i] = Complex(signal_real[i], signal_imag[i]);
        }
    } else {
        my_signal.resize(buffer_len_TX);
        for (int i = 0; i < buffer_len_TX; ++i) {
            my_signal[i] = Complex(appended_real[i], appended_imag[i]);
        }
    }

    return my_signal;
}
