#include <iostream>

#include "barker_generator.h"

CVec barker_generator(const std::string& barker_type, const std::string& modulation_type) {
    CVec header;

    if (barker_type == "barker13") {
        double br[] = {1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1};
        double bi[] = {1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1};
        for (int i = 0; i < 13; ++i) {
            header.push_back(Complex(br[i], bi[i]));
        }
    } else if (barker_type == "barker11") {
        double br[] = {1, 1, 1, -1, -1, -1, 1, -1, -1, 1, -1};
        double bi[] = {1, 1, 1, -1, -1, -1, 1, -1, -1, 1, -1};
        for (int i = 0; i < 11; ++i) {
            header.push_back(Complex(br[i], bi[i]));
        }
    } else if (barker_type == "barker7") {
        double br[] = {1, 1, 1, -1, -1, 1, -1};
        double bi[] = {1, 1, 1, -1, -1, 1, -1};
        for (int i = 0; i < 7; ++i) {
            header.push_back(Complex(br[i], bi[i]));
        }
    } else {
        std::cerr << "Input header" << std::endl;
    }

    if (modulation_type == "QAM4_2") {
        for (auto& h : header) {
            h *= 3.0;
        }
    } else if (modulation_type == "4PAM") {
        for (auto& h : header) {
            h = Complex(h.real() * 3.0, 0.0);
        }
    } else {
        for (auto& h : header) {
            h *= 1.1;
        }
    }

    return header;
}
