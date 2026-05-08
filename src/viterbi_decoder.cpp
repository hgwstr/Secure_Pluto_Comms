#include <vector>
#include <string>
#include <algorithm>
#include <climits>

class ViterbiDecoder {
public:
    static const int K = 7;
    static const int num_states = 64;
    
    std::string decode(const std::vector<int>& coded_bits) {
        int n = coded_bits.size();
        int num_symbols = n / 2;
        
        std::vector<std::vector<int>> metrics(num_symbols + 1, std::vector<int>(num_states, 1e6));
        std::vector<std::vector<int>> predecessors(num_symbols + 1, std::vector<int>(num_states, 0));
        
        metrics[0][0] = 0;

        for (int t = 0; t < num_symbols; ++t) {
            int r1 = coded_bits[2 * t];
            int r2 = coded_bits[2 * t + 1];

            for (int s = 0; s < num_states; ++s) {
                if (metrics[t][s] >= 1e6) continue;

                for (int bit = 0; bit <= 1; ++bit) {
                    int next_state = ((s << 1) | bit) & 0x3F;
                    
                    int out1 = 0, out2 = 0;
                    int reg = (s << 1) | bit;
                    for (int j = 0; j < K; ++j) {
                        if ((0x79 >> j) & 1) out1 ^= (reg >> j) & 1;
                        if ((0x5B >> j) & 1) out2 ^= (reg >> j) & 1;
                    }

                    int branch_metric = (r1!= out1) + (r2!= out2);
                    int new_metric = metrics[t][s] + branch_metric;

                    if (new_metric < metrics[t + 1][next_state]) {
                        metrics[t + 1][next_state] = new_metric;
                        predecessors[t + 1][next_state] = s;
                    }
                }
            }
        }


        std::vector<int> decoded_bits;
        int curr_state = 0;
        for (int t = num_symbols; t > 0; --t) {
            int prev_state = predecessors[t][curr_state];
            decoded_bits.push_back(curr_state & 1);
            curr_state = prev_state;
        }
        std::reverse(decoded_bits.begin(), decoded_bits.end());

        std::string result = "";
        for (size_t i = 0; i < decoded_bits.size(); i += 8) {
            unsigned char c = 0;
            for (int j = 0; j < 8 && (i + j) < decoded_bits.size(); ++j) {
                c |= (decoded_bits[i + j] << (7 - j));
            }
            if (c > 0) result += (char)c;
        }
        return result;
    }
};