#include "operation_tx.h"
#include "frame_generator_tx.h"
#include "signal_generator_tx.h"
#include <iostream>

void operation_TX(MyRadio& my_SDR, const std::string& msg, bool plotGraphs, bool info) {
    int buffer_length_TX = 1 << 18; // 2^18 = 262144

    // Pulse parameters
    int OVERSAMPLING_RATE = 16;
    int HALF_NO_OF_SYMBOLS = 6;
    double ROLLOFF_FACTOR = 0.75;
    int DATA_LENGTH = 80;

    auto [my_frames, single_frame_length, my_header, data_len_with_id] =
        frame_generator_TX(DATA_LENGTH, msg, "barker13", "QAM4_2", false);

    CVec my_signal = signal_generator(my_frames, buffer_length_TX,
                                       OVERSAMPLING_RATE, HALF_NO_OF_SYMBOLS,
                                       ROLLOFF_FACTOR, 1);

    // Truncate to buffer length
    if (static_cast<int>(my_signal.size()) > buffer_length_TX) {
        my_signal.resize(buffer_length_TX);
    }

    if (info) {
        std::cout << "Appended Symbol Frames Length = " << my_frames.size() << std::endl;
        std::cout << "Message Character Length = " << msg.size() << std::endl;
        std::cout << "Signal length = " << my_signal.size() << std::endl;
    }

    my_SDR.transmit_samples(my_signal);
}
