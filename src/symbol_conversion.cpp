#include "symbol_conversion.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <string>

// Вспомогательная функция для перевода числа в base-4 (не требуется в кодер, оставлена)
static std::string to_base4(int val) {
    if (val == 0) return "0000";
    std::string result;
    while (val > 0) {
        result += static_cast<char>('0' + (val % 4));
        val /= 4;
    }
    std::reverse(result.begin(), result.end());
    while (result.size() < 4) {
        result = "0" + result;
    }
    return result;
}

std::vector<int> letters_to_pam(const std::string& text) {
    // Параметры свёрточного кодера (аналогично ViterbiDecoder)
    const int G1 = 0x79;
    const int G2 = 0x5B;
    const int K = 7;

    std::vector<int> encoded_bits;
    unsigned int shift_register = 0;

    for (unsigned char c : text) {
        for (int i = 7; i >= 0; --i) {
            int bit = (c >> i) & 1;

            shift_register = (shift_register << 1) | bit;

            int out1 = 0, out2 = 0;
            for (int j = 0; j < K; ++j) {
                if ((G1 >> j) & 1) out1 ^= (shift_register >> j) & 1;
                if ((G2 >> j) & 1) out2 ^= (shift_register >> j) & 1;
            }

            encoded_bits.push_back(out1);
            encoded_bits.push_back(out2);

            shift_register &= 0x3F;   // оставляем 6 младших бит
        }
    }

    std::vector<int> symbols_PAM;
    for (size_t i = 0; i < encoded_bits.size(); i += 2) {
        int quaternary = (encoded_bits[i] << 1) | encoded_bits[i + 1];
        symbols_PAM.push_back(2 * quaternary - 3);
    }
    return symbols_PAM;
}

std::string pam_to_letters(const std::vector<int>& symbols_PAM) {
    // Заглушка – реализуйте при необходимости
    (void)symbols_PAM;
    return "";
}

CVec pam_to_qam4_2(const std::vector<int>& symbols_PAM) {
    CVec result(symbols_PAM.size());
    for (size_t i = 0; i < symbols_PAM.size(); ++i) {
        switch (symbols_PAM[i]) {
            case -3: result[i] = Complex(-3.0, -3.0); break;
            case -1: result[i] = Complex(-1.0, -1.0); break;
            case 1:  result[i] = Complex(1.0, 1.0);   break;
            default: result[i] = Complex(3.0, 3.0);   break;
        }
    }
    return result;
}

std::vector<int> qam4_2_to_pam(const CVec& symbols_QAM4_2) {
    std::vector<int> result(symbols_QAM4_2.size());
    for (size_t i = 0; i < symbols_QAM4_2.size(); ++i) {
        Complex s = symbols_QAM4_2[i];
        if (s.real() == -3.0 && s.imag() == -3.0)      result[i] = -3;
        else if (s.real() == -1.0 && s.imag() == -1.0) result[i] = -1;
        else if (s.real() == 1.0 && s.imag() == 1.0)   result[i] = 1;
        else                                            result[i] = 3;
    }
    return result;
}

CVec pam_to_qam(const std::vector<int>& symbols_PAM) {
    CVec result(symbols_PAM.size());
    for (size_t i = 0; i < symbols_PAM.size(); ++i) {
        switch (symbols_PAM[i]) {
            case -3: result[i] = Complex(-1.0, -1.0); break;
            case -1: result[i] = Complex(-1.0, 1.0);  break;
            case 1:  result[i] = Complex(1.0, -1.0);  break;
            default: result[i] = Complex(1.0, 1.0);   break;
        }
    }
    return result;
}

std::vector<int> qam_to_pam(const CVec& symbols_QAM) {
    std::vector<int> result(symbols_QAM.size());
    for (size_t i = 0; i < symbols_QAM.size(); ++i) {
        Complex s = symbols_QAM[i];
        if (s.real() == -1.0 && s.imag() == -1.0)      result[i] = -3;
        else if (s.real() == -1.0 && s.imag() == 1.0)  result[i] = -1;
        else if (s.real() == 1.0 && s.imag() == -1.0)  result[i] = 1;
        else                                            result[i] = 3;
    }
    return result;
}