#include "my_radio.h"
#include <iio.h>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <iostream>
#include <cmath>

MyRadio::MyRadio(const std::string& uri, const std::string& user_name,
                 int tx_length, bool ongoing_transmission)
    : uri_("ip:" + uri)
    , user_name_(user_name)
    , tx_length_(tx_length)
    , ongoing_transmission_(ongoing_transmission)
    , rx_buffer_size_(1 << 16)  // 2^16 default
    , tx_cyclic_buffer_(false)
    , ctx_(nullptr)
    , phy_dev_(nullptr)
    , tx_dev_(nullptr)
    , rx_dev_(nullptr)
    , tx_chan_i_(nullptr)
    , tx_chan_q_(nullptr)
    , rx_chan_i_(nullptr)
    , rx_chan_q_(nullptr)
    , tx_buf_(nullptr)
    , rx_buf_(nullptr)
{
    ctx_ = iio_create_context_from_uri(uri_.c_str());
    if (!ctx_) {
        throw std::runtime_error("Could not create IIO context for URI: " + uri_);
    }

    // Find devices
    phy_dev_ = iio_context_find_device(ctx_, "ad9361-phy");
    tx_dev_ = iio_context_find_device(ctx_, "cf-ad9361-dds-core-lpc");
    rx_dev_ = iio_context_find_device(ctx_, "cf-ad9361-lpc");

    if (!phy_dev_ || !tx_dev_ || !rx_dev_) {
        iio_context_destroy(ctx_);
        throw std::runtime_error("Could not find required IIO devices");
    }

    setup_channels();
}

MyRadio::~MyRadio() {
    if (tx_buf_) {
        iio_buffer_destroy(tx_buf_);
        tx_buf_ = nullptr;
    }
    if (rx_buf_) {
        iio_buffer_destroy(rx_buf_);
        rx_buf_ = nullptr;
    }
    if (ctx_) {
        iio_context_destroy(ctx_);
        ctx_ = nullptr;
    }
}

void MyRadio::setup_channels() {
    // TX channels
    tx_chan_i_ = iio_device_find_channel(tx_dev_, "voltage0", true);
    tx_chan_q_ = iio_device_find_channel(tx_dev_, "voltage1", true);
    if (tx_chan_i_) iio_channel_enable(tx_chan_i_);
    if (tx_chan_q_) iio_channel_enable(tx_chan_q_);

    // RX channels
    rx_chan_i_ = iio_device_find_channel(rx_dev_, "voltage0", false);
    rx_chan_q_ = iio_device_find_channel(rx_dev_, "voltage1", false);
    if (rx_chan_i_) iio_channel_enable(rx_chan_i_);
    if (rx_chan_q_) iio_channel_enable(rx_chan_q_);
}

void MyRadio::wr_ch_lli(iio_channel* ch, const char* attr, long long val) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%lld", val);
    int ret = iio_channel_attr_write(ch, attr, buf);
    if (ret < 0) {
        std::cerr << "Warning: failed to write " << attr << " = " << val << std::endl;
    }
}

void MyRadio::wr_ch_str(iio_channel* ch, const char* attr, const char* val) {
    int ret = iio_channel_attr_write(ch, attr, val);
    if (ret < 0) {
        std::cerr << "Warning: failed to write " << attr << " = " << val << std::endl;
    }
}

long long MyRadio::rd_ch_lli(iio_channel* ch, const char* attr) const {
    char buf[64];
    int ret = iio_channel_attr_read(ch, attr, buf, sizeof(buf));
    if (ret < 0) return 0;
    return std::strtoll(buf, nullptr, 10);
}

std::string MyRadio::rd_ch_str(iio_channel* ch, const char* attr) const {
    char buf[256];
    int ret = iio_channel_attr_read(ch, attr, buf, sizeof(buf));
    if (ret < 0) return "";
    return std::string(buf);
}

// Getters
long long MyRadio::rx_lo() const {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "altvoltage0", true);
    return rd_ch_lli(ch, "frequency");
}

long long MyRadio::tx_lo() const {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "altvoltage1", true);
    return rd_ch_lli(ch, "frequency");
}

long long MyRadio::sample_rate() const {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", false);
    return rd_ch_lli(ch, "sampling_frequency");
}

long long MyRadio::rx_rf_bandwidth() const {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", false);
    return rd_ch_lli(ch, "rf_bandwidth");
}

long long MyRadio::tx_rf_bandwidth() const {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", true);
    return rd_ch_lli(ch, "rf_bandwidth");
}

double MyRadio::tx_hardwaregain_chan0() const {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", true);
    char buf[64];
    iio_channel_attr_read(ch, "hardwaregain", buf, sizeof(buf));
    return std::strtod(buf, nullptr);
}

double MyRadio::rx_hardwaregain_chan0() const {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", false);
    char buf[64];
    iio_channel_attr_read(ch, "hardwaregain", buf, sizeof(buf));
    return std::strtod(buf, nullptr);
}

std::string MyRadio::gain_control_mode_chan0() const {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", false);
    return rd_ch_str(ch, "gain_control_mode");
}

int MyRadio::loopback() const {
    char buf[64];
    iio_device_attr_read(phy_dev_, "loopback", buf, sizeof(buf));
    return std::strtol(buf, nullptr, 10);
}

// Setters
void MyRadio::set_rx_lo(long long freq) {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "altvoltage0", true);
    wr_ch_lli(ch, "frequency", freq);
}

void MyRadio::set_tx_lo(long long freq) {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "altvoltage1", true);
    wr_ch_lli(ch, "frequency", freq);
}

void MyRadio::set_sample_rate(long long rate) {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", false);
    wr_ch_lli(ch, "sampling_frequency", rate);
}

void MyRadio::set_rx_rf_bandwidth(long long bw) {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", false);
    wr_ch_lli(ch, "rf_bandwidth", bw);
}

void MyRadio::set_tx_rf_bandwidth(long long bw) {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", true);
    wr_ch_lli(ch, "rf_bandwidth", bw);
}

void MyRadio::set_tx_hardwaregain_chan0(double gain) {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", true);
    char buf[64];
    snprintf(buf, sizeof(buf), "%.6f dB", gain);
    iio_channel_attr_write(ch, "hardwaregain", buf);
}

void MyRadio::set_rx_hardwaregain_chan0(double gain) {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", false);
    char buf[64];
    snprintf(buf, sizeof(buf), "%.6f dB", gain);
    iio_channel_attr_write(ch, "hardwaregain", buf);
}

void MyRadio::set_gain_control_mode_chan0(const std::string& mode) {
    iio_channel* ch = iio_device_find_channel(phy_dev_, "voltage0", false);
    wr_ch_str(ch, "gain_control_mode", mode.c_str());
}

void MyRadio::set_rx_buffer_size(int size) {
    rx_buffer_size_ = size;
    // Recreate RX buffer if it exists
    if (rx_buf_) {
        iio_buffer_destroy(rx_buf_);
        rx_buf_ = nullptr;
    }
}

void MyRadio::set_loopback(int mode) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d", mode);
    iio_device_attr_write(phy_dev_, "loopback", buf);
}

void MyRadio::destroy_tx_buffer() {
    if (tx_buf_) {
        iio_buffer_destroy(tx_buf_);
        tx_buf_ = nullptr;
    }
    ongoing_transmission_ = false;
}

void MyRadio::transmit_samples(const std::vector<std::complex<double>>& samples) {
    if (tx_cyclic_buffer_ && ongoing_transmission_) {
        kill_transmission();
    }

    tx_cyclic_buffer_ = true;

    // Create cyclic TX buffer
    int num_samples = static_cast<int>(samples.size());
    tx_buf_ = iio_device_create_buffer(tx_dev_, num_samples, true);
    if (!tx_buf_) {
        std::cerr << "Error: Could not create TX buffer" << std::endl;
        return;
    }

    // Fill buffer with scaled samples (scale by 2^14)
    const double scale = static_cast<double>(1 << 14);
    char* buf_start = static_cast<char*>(iio_buffer_start(tx_buf_));
    char* buf_end = static_cast<char*>(iio_buffer_end(tx_buf_));

    ptrdiff_t buf_step = iio_buffer_step(tx_buf_);
    char* ptr = buf_start;

    for (int i = 0; i < num_samples && ptr < buf_end; ++i) {
        int16_t i_val = static_cast<int16_t>(std::round(samples[i].real() * scale));
        int16_t q_val = static_cast<int16_t>(std::round(samples[i].imag() * scale));

        // Write I and Q interleaved (16-bit each)
        reinterpret_cast<int16_t*>(ptr)[0] = i_val;
        reinterpret_cast<int16_t*>(ptr)[1] = q_val;
        ptr += buf_step;
    }

    // Push buffer (cyclic buffers auto-repeat)
    iio_buffer_push(tx_buf_);
    ongoing_transmission_ = true;
}

void MyRadio::kill_transmission() {
    destroy_tx_buffer();
}

std::vector<std::complex<double>> MyRadio::receive_samples() {
    int num_samples = rx_buffer_size_;

    // Receive 5 times to flush pipeline (matching Python behavior)
    std::vector<std::complex<double>> result(num_samples);

    for (int flush = 0; flush < 5; ++flush) {
        // Create RX buffer
        if (rx_buf_) {
            iio_buffer_destroy(rx_buf_);
            rx_buf_ = nullptr;
        }
        rx_buf_ = iio_device_create_buffer(rx_dev_, num_samples, false);
        if (!rx_buf_) {
            std::cerr << "Error: Could not create RX buffer" << std::endl;
            return result;
        }

        // Refill buffer
        ssize_t nbytes = iio_buffer_refill(rx_buf_);
        if (nbytes < 0) {
            std::cerr << "Error: RX buffer refill failed" << std::endl;
            return result;
        }

        // Read samples
        char* buf_start = static_cast<char*>(iio_buffer_start(rx_buf_));
        ptrdiff_t buf_step = iio_buffer_step(rx_buf_);
        char* ptr = buf_start;

        for (int i = 0; i < num_samples; ++i) {
            int16_t i_val = reinterpret_cast<int16_t*>(ptr)[0];
            int16_t q_val = reinterpret_cast<int16_t*>(ptr)[1];
            result[i] = std::complex<double>(
                static_cast<double>(i_val),
                static_cast<double>(q_val));
            ptr += buf_step;
        }
    }

    // Scale down by 2^11 (matching Python: samples / 2**11)
    const double scale = 1.0 / static_cast<double>(1 << 11);
    for (auto& s : result) {
        s *= scale;
    }

    return result;
}

std::string MyRadio::repr() const {
    std::ostringstream oss;
    oss << "Pluto(uri=\"" << uri_ << "\") object for user \"" << user_name_
        << "\" with following key properties:\n\n";

    oss << "rx_lo:                   " << (rx_lo() / 1000000.0) << "       MHz, Carrier frequency of RX path\n";
    oss << "rx_hardwaregain_chan0    " << rx_hardwaregain_chan0() << "       dB, Gain applied to RX path\n";
    oss << "rx_rf_bandwidth:         " << (rx_rf_bandwidth() / 1000000.0) << "       MHz, Bandwidth of front-end analog filter of RX path\n";
    oss << "gain_control_mode_chan0: " << gain_control_mode_chan0() << "       Receive path AGC Options: slow_attack, fast_attack, manual\n";
    oss << "rx_buffer_size:          " << rx_buffer_size_ << " Samples for receive\n\n";

    oss << "tx_lo:                   " << (tx_lo() / 1000000.0) << "       MHz, Carrier frequency of TX path\n";
    oss << "tx_hardwaregain_chan0:   " << tx_hardwaregain_chan0() << "       dB, Attenuation applied to TX path\n";
    oss << "tx_rf_bandwidth:         " << (tx_rf_bandwidth() / 1000000.0) << "       MHz, Bandwidth of front-end analog filter of TX path\n";
    oss << "tx_cyclic_buffer:        " << (tx_cyclic_buffer_ ? "True" : "False") << "       Toggles cyclic buffer\n";
    oss << "tx_length                " << tx_length_ << " Samples for transmission\n\n";

    oss << "sample_rate:             " << (sample_rate() / 1000000.0) << "       MSPS, Sample rate RX and TX paths\n";
    oss << "loopback:                " << loopback() << "       0=Disabled, 1=Digital, 2=RF\n\n";

    return oss.str();
}
