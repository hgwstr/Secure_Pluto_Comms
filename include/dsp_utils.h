#ifndef DSP_UTILS_H
#define DSP_UTILS_H

#include <complex>
#include <vector>

using Complex = std::complex<double>;
using CVec = std::vector<Complex>;
using DVec = std::vector<double>;

// Convolution modes
enum class ConvMode { FULL, SAME };

// Real-valued convolution
DVec convolve(const DVec& a, const DVec& b, ConvMode mode = ConvMode::FULL);

// Complex-valued convolution
CVec convolve_complex(const CVec& a, const CVec& b, ConvMode mode = ConvMode::FULL);

// Real cross-correlation (equivalent to numpy.correlate with mode='full')
DVec correlate(const DVec& a, const DVec& b, ConvMode mode = ConvMode::FULL);

// FFT and IFFT using FFTW3
CVec fft(const CVec& x);
CVec ifft(const CVec& x);

// FFT of real data
CVec fft_real(const DVec& x);

// fftshift (swap halves)
CVec fftshift(const CVec& x);

// Linspace
DVec linspace(double start, double stop, int num, bool endpoint = true);

// Arange
DVec arange(double start, double stop, double step);

#endif
