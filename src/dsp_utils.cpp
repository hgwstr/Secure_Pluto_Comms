#include <cmath>
#include <algorithm>

#include "dsp_utils.h"
#include <fftw3.h>

DVec convolve(const DVec& a, const DVec& b, ConvMode mode) {
    int na = static_cast<int>(a.size());
    int nb = static_cast<int>(b.size());
    int nc = na + nb - 1;
    DVec result(nc, 0.0);

    for (int n = 0; n < nc; ++n) {
        double sum = 0.0;
        int k_min = std::max(0, n - nb + 1);
        int k_max = std::min(na - 1, n);
        for (int k = k_min; k <= k_max; ++k) {
            sum += a[k] * b[n - k];
        }
        result[n] = sum;
    }

    if (mode == ConvMode::SAME) {
        int start = (nc - na) / 2;
        return DVec(result.begin() + start, result.begin() + start + na);
    }
    return result;
}

CVec convolve_complex(const CVec& a, const CVec& b, ConvMode mode) {
    int na = static_cast<int>(a.size());
    int nb = static_cast<int>(b.size());
    int nc = na + nb - 1;
    CVec result(nc, Complex(0.0, 0.0));

    for (int n = 0; n < nc; ++n) {
        Complex sum(0.0, 0.0);
        int k_min = std::max(0, n - nb + 1);
        int k_max = std::min(na - 1, n);
        for (int k = k_min; k <= k_max; ++k) {
            sum += a[k] * b[n - k];
        }
        result[n] = sum;
    }

    if (mode == ConvMode::SAME) {
        int start = (nc - na) / 2;
        return CVec(result.begin() + start, result.begin() + start + na);
    }
    return result;
}

DVec correlate(const DVec& a, const DVec& b, ConvMode mode) {
    // numpy.correlate(a, b, 'full') is equivalent to convolve(a, reverse(b))
    int nb = static_cast<int>(b.size());
    DVec b_rev(nb);
    for (int i = 0; i < nb; ++i) {
        b_rev[i] = b[nb - 1 - i];
    }
    return convolve(a, b_rev, mode);
}

CVec fft(const CVec& x) {
    int N = static_cast<int>(x.size());
    CVec result(N);

    fftw_complex* in = reinterpret_cast<fftw_complex*>(
        fftw_malloc(sizeof(fftw_complex) * N));
    fftw_complex* out = reinterpret_cast<fftw_complex*>(
        fftw_malloc(sizeof(fftw_complex) * N));

    for (int i = 0; i < N; ++i) {
        in[i][0] = x[i].real();
        in[i][1] = x[i].imag();
    }

    fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    for (int i = 0; i < N; ++i) {
        result[i] = Complex(out[i][0], out[i][1]);
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    return result;
}

CVec ifft(const CVec& x) {
    int N = static_cast<int>(x.size());
    CVec result(N);

    fftw_complex* in = reinterpret_cast<fftw_complex*>(
        fftw_malloc(sizeof(fftw_complex) * N));
    fftw_complex* out = reinterpret_cast<fftw_complex*>(
        fftw_malloc(sizeof(fftw_complex) * N));

    for (int i = 0; i < N; ++i) {
        in[i][0] = x[i].real();
        in[i][1] = x[i].imag();
    }

    fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    for (int i = 0; i < N; ++i) {
        result[i] = Complex(out[i][0] / N, out[i][1] / N);
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    return result;
}

CVec fft_real(const DVec& x) {
    int N = static_cast<int>(x.size());
    CVec cx(N);
    for (int i = 0; i < N; ++i) {
        cx[i] = Complex(x[i], 0.0);
    }
    return fft(cx);
}

CVec fftshift(const CVec& x) {
    int N = static_cast<int>(x.size());
    CVec result(N);
    int half = N / 2;
    for (int i = 0; i < N; ++i) {
        result[i] = x[(i + half) % N];
    }
    return result;
}

DVec linspace(double start, double stop, int num, bool endpoint) {
    DVec result(num);
    if (num == 1) {
        result[0] = start;
        return result;
    }
    double step = endpoint ? (stop - start) / (num - 1) : (stop - start) / num;
    for (int i = 0; i < num; ++i) {
        result[i] = start + i * step;
    }
    return result;
}

DVec arange(double start, double stop, double step) {
    DVec result;
    for (double v = start; v < stop; v += step) {
        result.push_back(v);
    }
    return result;
}
