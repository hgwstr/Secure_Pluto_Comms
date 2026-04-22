#include "symbol_conversion.h"
#include <cmath>
#include <sstream>
#include <algorithm>

// Helper: convert integer to base-4 string representation, padded to 4 chars
static std::string to_base4(int val) {
    if (val == 0) return "0000";
    std::string result;
    while (val > 0) {
        result += static_cast<char>('0' + (val % 4));
        val /= 4;
    }
    std::reverse(result.begin(), result.end());
    // Pad to 4 digits
    while (result.size() < 4) {
        result = "0" + result;
    }
    return result;
}

std::vector<int> letters_to_pam(const std::string& text) {
    std::string msg;
    for (char c : text) {
        int ord_c = static_cast<int>(static_cast<unsigned char>(c));
        std::string letter = to_base4(ord_c);
        msg += letter;
    }

    std::vector<int> symbols_PAM(msg.size());
    for (size_t i = 0; i < msg.size(); ++i) {
        symbols_PAM[i] = 2 * static_cast<int>(msg[i]) - 99;
    }
    return symbols_PAM;
}

std::string pam_to_letters(const std::vector<int>& symbols_PAM) {
    int N = static_cast<int>(symbols_PAM.size());
    int off = N % 4;
    int use_len = (off != 0) ? N - off : N;

    std::string my_4base;
    std::string converted_string;

    for (int i = 0; i < use_len; ++i) {
        int symbol = symbols_PAM[i];
        char ch = static_cast<char>((symbol + 99) / 2);
        my_4base += ch;
        if (my_4base.size() == 4) {
            // Convert base-4 string to character
            int val = 0;
            for (char d : my_4base) {
                val = val * 4 + (d - '0');
            }
            converted_string += static_cast<char>(val);
            my_4base.clear();
        }
    }
    return converted_string;
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
        else if (s.real() == -1.0 && s.imag() == -1.0)  result[i] = -1;
        else if (s.real() == 1.0 && s.imag() == 1.0)    result[i] = 1;
        else                                              result[i] = 3;
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
        else if (s.real() == -1.0 && s.imag() == 1.0)   result[i] = -1;
        else if (s.real() == 1.0 && s.imag() == -1.0)   result[i] = 1;
        else                                              result[i] = 3;
    }
    return result;
}
