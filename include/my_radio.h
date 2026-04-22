#ifndef MY_RADIO_H
#define MY_RADIO_H

#include <string>
#include <vector>
#include <complex>
#include <cstdint>

struct iio_context;
struct iio_device;
struct iio_channel;
struct iio_buffer;

class MyRadio {
public:
    MyRadio(const std::string& uri, const std::string& user_name,
            int tx_length = (1 << 18), bool ongoing_transmission = false);
    ~MyRadio();

    // Non-copyable
    MyRadio(const MyRadio&) = delete;
    MyRadio& operator=(const MyRadio&) = delete;

    // Properties
    std::string user_name() const { return user_name_; }
    void set_user_name(const std::string& name) { user_name_ = name; }

    int tx_length() const { return tx_length_; }
    void set_tx_length(int len) { tx_length_ = len; }

    bool ongoing_transmission() const { return ongoing_transmission_; }

    // SDR parameter getters
    long long rx_lo() const;
    long long tx_lo() const;
    long long sample_rate() const;
    long long rx_rf_bandwidth() const;
    long long tx_rf_bandwidth() const;
    double tx_hardwaregain_chan0() const;
    double rx_hardwaregain_chan0() const;
    std::string gain_control_mode_chan0() const;
    int rx_buffer_size() const { return rx_buffer_size_; }
    int loopback() const;

    // SDR parameter setters
    void set_rx_lo(long long freq);
    void set_tx_lo(long long freq);
    void set_sample_rate(long long rate);
    void set_rx_rf_bandwidth(long long bw);
    void set_tx_rf_bandwidth(long long bw);
    void set_tx_hardwaregain_chan0(double gain);
    void set_rx_hardwaregain_chan0(double gain);
    void set_gain_control_mode_chan0(const std::string& mode);
    void set_rx_buffer_size(int size);
    void set_loopback(int mode);

    // TX/RX operations
    void transmit_samples(const std::vector<std::complex<double>>& samples);
    void kill_transmission();
    std::vector<std::complex<double>> receive_samples();

    // Display
    std::string repr() const;

private:
    std::string uri_;
    std::string user_name_;
    int tx_length_;
    bool ongoing_transmission_;
    int rx_buffer_size_;
    bool tx_cyclic_buffer_;

    // libiio handles
    iio_context* ctx_;
    iio_device* phy_dev_;
    iio_device* tx_dev_;
    iio_device* rx_dev_;
    iio_channel* tx_chan_i_;
    iio_channel* tx_chan_q_;
    iio_channel* rx_chan_i_;
    iio_channel* rx_chan_q_;
    iio_buffer* tx_buf_;
    iio_buffer* rx_buf_;

    // Helper to write channel attributes
    void wr_ch_lli(iio_channel* ch, const char* attr, long long val);
    void wr_ch_str(iio_channel* ch, const char* attr, const char* val);
    long long rd_ch_lli(iio_channel* ch, const char* attr) const;
    std::string rd_ch_str(iio_channel* ch, const char* attr) const;

    void setup_channels();
    void destroy_tx_buffer();
};

#endif
