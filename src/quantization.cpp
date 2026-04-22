#include "quantization.h"
#include <cmath>
#include <algorithm>

std::pair<CVec, CVec> quantalph_distance(const CVec& symbol_array,
                                          const DVec& alphabet) {
    double min_alphabet = *std::min_element(alphabet.begin(), alphabet.end());
    double max_alphabet = *std::max_element(alphabet.begin(), alphabet.end());

    // Find min and max of the symbol array (by real part for complex min/max)
    // Python: min_point = min(symbol_array), max_point = max(symbol_array)
    // numpy min/max on complex compares real parts first, then imag
    // But the Python code uses abs(min_point) + abs(max_point) where abs is complex magnitude
    // Actually looking more carefully: min/max returns the element with smallest/largest
    // real part. But then abs() is complex abs.
    double min_abs = std::abs(symbol_array[0]);
    double max_abs = std::abs(symbol_array[0]);
    for (const auto& s : symbol_array) {
        double a = std::abs(s);
        if (a < min_abs) min_abs = a;
        if (a > max_abs) max_abs = a;
    }

    // Actually, re-reading the Python code:
    // min_point = min(symbol_array) -- numpy min on complex sorts by real then imag
    // max_point = max(symbol_array)
    // distance = abs(min_point) + abs(max_point)
    // Let me match the exact Python behavior:
    // np.min on complex array returns element with smallest real part (then imag as tiebreaker)
    Complex min_point = symbol_array[0];
    Complex max_point = symbol_array[0];
    for (const auto& s : symbol_array) {
        if (s.real() < min_point.real() ||
            (s.real() == min_point.real() && s.imag() < min_point.imag())) {
            min_point = s;
        }
        if (s.real() > max_point.real() ||
            (s.real() == max_point.real() && s.imag() > max_point.imag())) {
            max_point = s;
        }
    }
    double distance = std::abs(min_point) + std::abs(max_point);
    double threshold = 1.0 * distance / 4.0;

    int N = static_cast<int>(symbol_array.size());
    CVec quantized_symbols(N);
    CVec threshold_array(N);

    for (int i = 0; i < N; ++i) {
        double angle = 2.0 * M_PI * static_cast<double>(i) / N;
        threshold_array[i] = threshold * Complex(std::cos(angle), std::sin(angle));
    }

    for (int i = 0; i < N; ++i) {
        double sign_real = (symbol_array[i].real() >= 0) ? 1.0 : -1.0;
        double sign_imag = (symbol_array[i].imag() >= 0) ? 1.0 : -1.0;
        // Handle exact zero: numpy sign(0) = 0
        if (symbol_array[i].real() == 0.0) sign_real = 0.0;
        if (symbol_array[i].imag() == 0.0) sign_imag = 0.0;

        if (std::abs(symbol_array[i]) < threshold) {
            quantized_symbols[i] = Complex(sign_real * min_alphabet,
                                           sign_imag * min_alphabet);
        } else {
            quantized_symbols[i] = Complex(sign_real * max_alphabet,
                                           sign_imag * max_alphabet);
        }
    }

    return {quantized_symbols, threshold_array};
}
