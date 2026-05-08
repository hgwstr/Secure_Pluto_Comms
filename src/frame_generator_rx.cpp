#include "../include/frame_generator_rx.h"
#include "../include/barker_generator.h"
#include "../include/symbol_conversion.h"
#include "../include/viterbi_decoder.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <complex>
#include "../include/dsp_utils.h" 

std::string frame_generator_RX(const CVec& quantized_symbols, 
                               const std::pair<CVec, CVec>& correlation_data,
                               int single_frame_length, 
                               const std::string& modulation_type,
                               const std::string& header) {
    std::string full_decoded_msg = "";
    ViterbiDecoder decoder;
    
    const CVec& correlation_indices = correlation_data.first;
    const CVec& correlation_values = correlation_data.second;

    for (size_t idx = 0; idx < correlation_indices.size(); ++idx) {
        int start_idx = static_cast<int>(correlation_indices[idx].real());
        
        if (start_idx + single_frame_length > quantized_symbols.size()) continue;

        CVec frame_symbols(single_frame_length);
        double phase_corr_real = (correlation_values[idx].real() >= 0)? 1.0 : -1.0;
        double phase_corr_imag = (correlation_values[idx].imag() >= 0)? 1.0 : -1.0;

        for (int i = 0; i < single_frame_length; ++i) {
            double re = quantized_symbols[start_idx + i].real() * phase_corr_real;
            double im = quantized_symbols[start_idx + i].imag() * phase_corr_imag;
            frame_symbols[i] = std::complex<double>(re, im);
        }

        std::vector<int> pam_levels;
        if (modulation_type == "QAM4_2") {
            pam_levels = qam4_2_to_pam(frame_symbols);
        } else {
            pam_levels = qam_to_pam(frame_symbols);
        }

        std::vector<int> bits_to_decode;
        for (int val : pam_levels) {
            int quaternary = (val + 3) / 2;
            bits_to_decode.push_back((quaternary >> 1) & 1);
            bits_to_decode.push_back(quaternary & 1);
        }

        std::string raw_frame_text = decoder.decode(bits_to_decode);

        full_decoded_msg += raw_frame_text;
    }

    return full_decoded_msg;
}