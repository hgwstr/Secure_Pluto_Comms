#include "operation_rx.h"
#include "pulse_shape.h"
#include "maximum_frequency.h"
#include "costas_loop.h"
#include "clock_recovery.h"
#include "quantization.h"
#include "symbol_correlation.h"
#include "frame_generator_rx.h"
#include "message_handler.h"
#include "dsp_utils.h"
#include <cmath>
#include <algorithm>
#include <iostream>

std::string operation_RX(MyRadio& my_SDR, bool plot_graphs) {
    // Correlation parameters
    double TRIGGER = 80.0;
    int HEADER_LENGTH = 13;
    int FRAME_LENGTH_SYMBOLS = 365;

    // SRRC parameters
    int OVERSAMPLING_RATE = 16;
    int HALF_NO_OF_SYMBOLS = 6;
    double ROLLOFF_FACTOR = 0.75;

    double sampling_rate = static_cast<double>(my_SDR.sample_rate());
    double Ts = 1.0 / sampling_rate;

    DVec pulse = srrc(HALF_NO_OF_SYMBOLS, ROLLOFF_FACTOR, OVERSAMPLING_RATE);

    // ++++++++++++++++++++ RX ++++++++++++++++++++
    CVec rx = my_SDR.receive_samples();
    int N = static_cast<int>(rx.size());

    // Time vector
    DVec t(N);
    for (int i = 0; i < N; ++i) {
        t[i] = i * Ts;
    }

    // COARSE FREQUENCY CORRECTION
    // r4th = rx^4
    CVec r4th(N);
    for (int i = 0; i < N; ++i) {
        Complex r = rx[i];
        Complex r2 = r * r;
        r4th[i] = r2 * r2;
    }

    double coarse_frequency = maximum_frequency(r4th, sampling_rate) / 4.0;

    // Demodulate
    CVec coarse_baseband(N);
    for (int i = 0; i < N; ++i) {
        Complex demod = std::exp(Complex(0.0, -2.0 * M_PI * coarse_frequency * t[i]));
        coarse_baseband[i] = rx[i] * demod;
    }

    // BAND SHIFTING BEFORE PHASE CORRECTION
    double fc = 2e6;
    CVec shifted_before_CL(N);
    for (int i = 0; i < N; ++i) {
        Complex shift = std::exp(Complex(0.0, 2.0 * M_PI * fc * t[i]));
        shifted_before_CL[i] = coarse_baseband[i] * shift;
    }

    // COSTAS LOOP
    DVec shifted_real(N);
    for (int i = 0; i < N; ++i) {
        shifted_real[i] = shifted_before_CL[i].real();
    }

    auto costas_result = costas_loop_QAM(shifted_real, sampling_rate, 0.2, fc, M_PI / 6.0);

    // Baseband signal = complex_exp_est * shifted_before_CL
    CVec baseband_signal(N);
    for (int i = 0; i < N; ++i) {
        baseband_signal[i] = costas_result.complex_exp_estimation[i] * shifted_before_CL[i];
    }

    // MATCHED FILTERING
    // convolve(baseband_signal, pulse, 'same') * max(pulse)
    // Need to convolve complex with real pulse
    // Convert pulse to complex for convolution
    DVec baseband_real(N), baseband_imag(N);
    for (int i = 0; i < N; ++i) {
        baseband_real[i] = baseband_signal[i].real();
        baseband_imag[i] = baseband_signal[i].imag();
    }

    DVec filtered_real = convolve(baseband_real, pulse, ConvMode::SAME);
    DVec filtered_imag = convolve(baseband_imag, pulse, ConvMode::SAME);

    double max_pulse = *std::max_element(pulse.begin(), pulse.end());

    int filt_len = static_cast<int>(filtered_real.size());
    CVec matched_filtered(filt_len);
    for (int i = 0; i < filt_len; ++i) {
        matched_filtered[i] = Complex(filtered_real[i] * max_pulse,
                                       filtered_imag[i] * max_pulse);
    }

    // CLOCK RECOVERY - WITH OUTPUT POWER MAXIMIZATION
    double tnow = 2.0 * HALF_NO_OF_SYMBOLS * OVERSAMPLING_RATE;

    DVec mf_real(filt_len), mf_imag(filt_len);
    for (int i = 0; i < filt_len; ++i) {
        mf_real[i] = matched_filtered[i].real();
        mf_imag[i] = matched_filtered[i].imag();
    }

    auto [tau1, downsampled_real] = clock_recovery_OP_max(
        mf_real, tnow, HALF_NO_OF_SYMBOLS, OVERSAMPLING_RATE,
        0.6, 2.0, ROLLOFF_FACTOR);

    auto [tau2, downsampled_imag] = clock_recovery_OP_max(
        mf_imag, tnow, HALF_NO_OF_SYMBOLS, OVERSAMPLING_RATE,
        0.6, 2.0, ROLLOFF_FACTOR);

    // Combine downsampled signal
    int ds_len = std::min(static_cast<int>(downsampled_real.size()),
                          static_cast<int>(downsampled_imag.size()));
    CVec downsampled_signal(ds_len);
    for (int i = 0; i < ds_len; ++i) {
        downsampled_signal[i] = Complex(downsampled_real[i], downsampled_imag[i]);
    }

    // QUANTIZATION
    DVec alphabet = {1.0, 3.0};
    auto [quantized_symbols, threshold] = quantalph_distance(downsampled_signal, alphabet);

    // CORRELATION
    auto [correlation_indices, correlation_values] = symbol_correlation(
        quantized_symbols, "QAM4_2", TRIGGER, HEADER_LENGTH);

    // MESSAGE GENERATION
    try {
        std::string received_message = frame_generator_RX(
            quantized_symbols, correlation_indices, correlation_values,
            FRAME_LENGTH_SYMBOLS, "barker13", "QAM4_2");

        std::string message_output = message_handler(received_message, false);
        return message_output;
    } catch (...) {
        return "";
    }
}
