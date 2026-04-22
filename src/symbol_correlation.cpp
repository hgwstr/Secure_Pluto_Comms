#include "symbol_correlation.h"
#include "barker_generator.h"
#include <cmath>
#include <sstream>

std::pair<CVec, CVec> symbol_correlation(const CVec& symbols,
                                          const std::string& modulation_type,
                                          double trigger,
                                          int header_length) {
    std::string barker_type = "barker" + std::to_string(header_length);
    CVec header = barker_generator(barker_type, modulation_type);

    // Separate real and imaginary parts
    int N = static_cast<int>(symbols.size());
    DVec real_symbols(N), imag_symbols(N);
    for (int i = 0; i < N; ++i) {
        real_symbols[i] = symbols[i].real();
        imag_symbols[i] = symbols[i].imag();
    }

    int hlen = static_cast<int>(header.size());
    DVec header_real(hlen), header_imag(hlen);
    for (int i = 0; i < hlen; ++i) {
        header_real[i] = header[i].real();
        header_imag[i] = header[i].imag();
    }

    // Correlate
    DVec correlation_real = correlate(real_symbols, header_real, ConvMode::FULL);
    DVec correlation_imag = correlate(imag_symbols, header_imag, ConvMode::FULL);

    // Find peaks above trigger
    const int MAX_CORR = 80;
    DVec real_corr_indices(MAX_CORR, 0.0);
    DVec real_corr_values(MAX_CORR, 0.0);
    DVec imag_corr_indices(MAX_CORR, 0.0);
    DVec imag_corr_values(MAX_CORR, 0.0);

    int ri = 0, ii = 0;

    for (int idx = 0; idx < static_cast<int>(correlation_real.size()); ++idx) {
        if (std::abs(correlation_real[idx]) >= trigger && ri < MAX_CORR) {
            real_corr_indices[ri] = static_cast<double>(idx);
            real_corr_values[ri] = correlation_real[idx];
            ri++;
        }
    }

    for (int idx = 0; idx < static_cast<int>(correlation_imag.size()); ++idx) {
        if (std::abs(correlation_imag[idx]) >= trigger && ii < MAX_CORR) {
            imag_corr_indices[ii] = static_cast<double>(idx);
            imag_corr_values[ii] = correlation_imag[idx];
            ii++;
        }
    }

    // Combine into complex: indices = real_indices + 1j*imag_indices
    CVec combined_indices(MAX_CORR);
    CVec combined_values(MAX_CORR);
    for (int i = 0; i < MAX_CORR; ++i) {
        combined_indices[i] = Complex(real_corr_indices[i], imag_corr_indices[i]);
        combined_values[i] = Complex(real_corr_values[i], imag_corr_values[i]);
    }

    // trim_zeros from back: remove trailing zeros
    int trim_end = MAX_CORR;
    while (trim_end > 0 &&
           combined_indices[trim_end - 1].real() == 0.0 &&
           combined_indices[trim_end - 1].imag() == 0.0) {
        trim_end--;
    }

    CVec trimmed_indices(combined_indices.begin(), combined_indices.begin() + trim_end);
    CVec trimmed_values(combined_values.begin(), combined_values.begin() + trim_end);

    return {trimmed_indices, trimmed_values};
}
