#include "frame_generator_rx.h"
#include "barker_generator.h"
#include "symbol_conversion.h"
#include <cmath>
#include <algorithm>

std::string frame_generator_RX(const CVec& quantized_symbols,
                                const CVec& correlation_indices,
                                const CVec& correlation_values,
                                int single_frame_length,
                                const std::string& header,
                                const std::string& modulation_type) {
    CVec header_vec = barker_generator(header, modulation_type);
    int header_len = static_cast<int>(header_vec.size());
    int frame_len = single_frame_length;
    std::string mod_type = modulation_type;
    std::string my_msg;

    int len_of_index_array = static_cast<int>(correlation_indices.size());

    for (int idx = 0; idx < len_of_index_array; ++idx) {
        int index_real = static_cast<int>(correlation_indices[idx].real());
        int index_imag = static_cast<int>(correlation_indices[idx].imag());
        double value_real = (correlation_values[idx].real() >= 0) ? 1.0 : -1.0;
        double value_imag = (correlation_values[idx].imag() >= 0) ? 1.0 : -1.0;
        // Handle zero: sign(0) = 0 in numpy
        if (correlation_values[idx].real() == 0.0) value_real = 0.0;
        if (correlation_values[idx].imag() == 0.0) value_imag = 0.0;

        int starting_index_real = index_real + 1;
        int starting_index_imag = index_imag + 1;
        int end_index_real, end_index_imag;

        if (idx != len_of_index_array - 1) {
            int next_index_real = static_cast<int>(correlation_indices[idx + 1].real());
            int next_index_imag = static_cast<int>(correlation_indices[idx + 1].imag());
            end_index_real = next_index_real - header_len + 1;
            end_index_imag = next_index_imag - header_len + 1;
        } else {
            end_index_real = starting_index_real + frame_len - header_len;
            end_index_imag = starting_index_imag + frame_len - header_len;
        }

        // Bounds checking
        int qs_len = static_cast<int>(quantized_symbols.size());
        if (end_index_real > qs_len) end_index_real = qs_len;
        if (end_index_imag > qs_len) end_index_imag = qs_len;
        if (starting_index_real >= qs_len || starting_index_imag >= qs_len) continue;

        // Extract symbols
        int sym_count_real = end_index_real - starting_index_real;
        int sym_count_imag = end_index_imag - starting_index_imag;
        int sym_count = std::min(sym_count_real, sym_count_imag);
        if (sym_count <= 0) continue;

        CVec symbols(sym_count);
        for (int i = 0; i < sym_count; ++i) {
            double sr = quantized_symbols[starting_index_real + i].real() * value_real;
            double si = quantized_symbols[starting_index_imag + i].imag() * value_imag;
            symbols[i] = Complex(sr, si);
        }

        // Convert to message
        std::string frame_msg;
        if (mod_type == "4QAM") {
            frame_msg = pam_to_letters(qam_to_pam(symbols));
        } else if (mod_type == "QAM4_2") {
            frame_msg = pam_to_letters(qam4_2_to_pam(symbols));
        } else {
            // Direct PAM (real parts only)
            std::vector<int> pam(sym_count);
            for (int i = 0; i < sym_count; ++i) {
                pam[i] = static_cast<int>(symbols[i].real());
            }
            frame_msg = pam_to_letters(pam);
        }

        my_msg += frame_msg;
    }

    return my_msg;
}
