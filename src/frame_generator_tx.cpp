#include "frame_generator_tx.h"
#include "symbol_conversion.h"
#include "barker_generator.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

TxFrameResult frame_generator_TX(int data_length,
                                  const std::string& text_message,
                                  const std::string& header_type,
                                  const std::string& modulation_type,
                                  bool info) {
    std::string mod_type = modulation_type;
    std::string msg = text_message;
    int single_frame_length = 0;
    CVec header = barker_generator(header_type, mod_type);
    int message_length = static_cast<int>(msg.size());
    int number_of_data_frames = message_length / data_length;
    int remainder = message_length % data_length;
    int symbols_per_char = 4;
    int id_length_in_char = 4;
    int id_len_in_symbols = id_length_in_char * symbols_per_char;
    int data_length_with_id = id_length_in_char + data_length + id_length_in_char;
    int last_data_length_with_id = id_length_in_char + remainder + id_length_in_char;

    CVec symbol_frames;

    const std::string encoding =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890";

    // Helper lambda to format hex ID like Python's f'{i:#04X}'
    // Python: f'{0:#04X}' = '0X00', f'{1:#04X}' = '0X01', f'{10:#04X}' = '0X0A'
    auto hex_id = [](int val) -> std::string {
        std::ostringstream oss;
        oss << "0X" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << val;
        return oss.str();
    };

    // Helper to convert string data to symbols
    auto to_symbols = [&](const std::string& data) -> CVec {
        std::vector<int> pam = letters_to_pam(data);
        if (mod_type == "4QAM") {
            return pam_to_qam(pam);
        } else if (mod_type == "QAM4_2") {
            return pam_to_qam4_2(pam);
        } else {
            // Return PAM as complex with zero imaginary
            CVec result(pam.size());
            for (size_t i = 0; i < pam.size(); ++i) {
                result[i] = Complex(static_cast<double>(pam[i]), 0.0);
            }
            return result;
        }
    };

    if (static_cast<int>(msg.size()) > data_length) {
        single_frame_length = static_cast<int>(header.size()) +
                              id_len_in_symbols +
                              data_length * symbols_per_char +
                              id_len_in_symbols;

        for (int i = 0; i <= number_of_data_frames; ++i) {
            std::string data;
            if (i == number_of_data_frames) {
                data = hex_id(i) +
                       msg.substr(i * data_length, remainder) +
                       hex_id(i);
                // Pad to data_length_with_id
                for (int j = 0; j < data_length_with_id - last_data_length_with_id; ++j) {
                    data += encoding[j];
                }
            } else {
                data = hex_id(i) +
                       msg.substr(i * data_length, data_length) +
                       hex_id(i);
            }

            if (info) {
                std::cout << "Data frame is generated: " << data << std::endl;
            }

            CVec data_symbols = to_symbols(data);

            // Prepend header
            CVec frame_with_header;
            frame_with_header.insert(frame_with_header.end(), header.begin(), header.end());
            frame_with_header.insert(frame_with_header.end(), data_symbols.begin(), data_symbols.end());

            // Append to symbol_frames
            symbol_frames.insert(symbol_frames.end(), frame_with_header.begin(), frame_with_header.end());
        }
    } else if (static_cast<int>(msg.size()) == data_length) {
        single_frame_length = static_cast<int>(header.size()) +
                              id_len_in_symbols +
                              data_length * symbols_per_char +
                              id_len_in_symbols;

        std::string data = hex_id(0) + msg + hex_id(0);
        if (info) {
            std::cout << data << std::endl;
        }

        CVec data_symbols = to_symbols(data);
        symbol_frames.insert(symbol_frames.end(), header.begin(), header.end());
        symbol_frames.insert(symbol_frames.end(), data_symbols.begin(), data_symbols.end());
    } else {
        // Message shorter than data_length
        single_frame_length = static_cast<int>(header.size()) +
                              id_len_in_symbols +
                              data_length * symbols_per_char +
                              id_len_in_symbols;

        std::string data = hex_id(0) + msg + hex_id(0);
        // Pad
        for (int i = 0; i < data_length - remainder; ++i) {
            data += encoding[i];
        }

        if (info) {
            std::cout << data << std::endl;
        }

        CVec data_symbols = to_symbols(data);
        symbol_frames.insert(symbol_frames.end(), header.begin(), header.end());
        symbol_frames.insert(symbol_frames.end(), data_symbols.begin(), data_symbols.end());
    }

    return {symbol_frames, single_frame_length, header, data_length_with_id};
}
