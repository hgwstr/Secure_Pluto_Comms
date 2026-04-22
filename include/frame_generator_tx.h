#ifndef FRAME_GENERATOR_TX_H
#define FRAME_GENERATOR_TX_H

#include "dsp_utils.h"
#include <string>
#include <tuple>

struct TxFrameResult {
    CVec symbol_frames;
    int single_frame_length;
    CVec header;
    int data_length_with_id;
};

// Generate symbol frames for transmission
TxFrameResult frame_generator_TX(int data_length,
                                  const std::string& text_message,
                                  const std::string& header_type,
                                  const std::string& modulation_type,
                                  bool info);

#endif
