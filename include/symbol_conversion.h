#ifndef SYMBOL_CONVERSION_H
#define SYMBOL_CONVERSION_H

#include "dsp_utils.h"
#include <string>
#include <vector>

// Convert text to PAM symbols (each char -> 4 PAM symbols)
std::vector<int> letters_to_pam(const std::string& text);

// Convert PAM symbols to text
std::string pam_to_letters(const std::vector<int>& symbols_PAM);

// PAM to QAM4_2 mapping: -3->(-3-3j), -1->(-1-1j), 1->(1+1j), 3->(3+3j)
CVec pam_to_qam4_2(const std::vector<int>& symbols_PAM);

// QAM4_2 to PAM mapping (reverse)
std::vector<int> qam4_2_to_pam(const CVec& symbols_QAM4_2);

// PAM to QAM mapping: -3->(-1-1j), -1->(-1+1j), 1->(1-1j), 3->(1+1j)
CVec pam_to_qam(const std::vector<int>& symbols_PAM);

// QAM to PAM mapping (reverse)
std::vector<int> qam_to_pam(const CVec& symbols_QAM);

#endif
