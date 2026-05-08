#ifndef VITERBI_DECODER_H
#define VITERBI_DECODER_H

#include <vector>
#include <string>
#include <algorithm>
#include <climits>

class ViterbiDecoder {
public:
    static const int K = 7;
    static const int num_states = 64;
    static const int G1 = 0x79;
    static const int G2 = 0x5B;

    ViterbiDecoder();

    std::string decode(const std::vector<int>& coded_bits);

private:
    std::pair<int, int> encode_bit(int current_state, int input_bit);
    static const int traceback_depth = 35; 
};

#endif