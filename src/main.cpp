#include "my_radio.h"
#include "operation_tx.h"
#include "operation_rx.h"

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <memory>
#include <csignal>

static const long long MHZ = 1000000LL;

static void transmit_message(const std::string& message, MyRadio& mySDR) {
    operation_TX(mySDR, message, false, false);
}

static void input_and_transmit(std::queue<std::string>& input_queue,
                                std::mutex& queue_mutex,
                                std::condition_variable& queue_cv,
                                std::atomic<bool>& exit_flag,
                                MyRadio& mySDR) {
    while (!exit_flag.load()) {
        std::string user_input;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (queue_cv.wait_for(lock, std::chrono::seconds(1),
                [&] { return !input_queue.empty() || exit_flag.load(); })) {
                if (exit_flag.load()) break;
                if (input_queue.empty()) continue;
                user_input = input_queue.front();
                input_queue.pop();
            } else {
                continue;
            }
        }

        if (user_input.empty()) continue;

        // Check for quit
        std::string lower = user_input;
        for (auto& c : lower) c = std::tolower(c);
        if (lower == "q") {
            exit_flag.store(true);
            break;
        }

        try {
            transmit_message(user_input, mySDR);
        } catch (const std::exception& e) {
            std::cerr << "TX Error: " << e.what() << std::endl;
        }
    }
}

static void print_message(std::atomic<bool>& exit_flag, MyRadio& mySDR) {
    while (!exit_flag.load()) {
        try {
            std::string received = operation_RX(mySDR, false);
            if (!received.empty()) {
                std::cout << "user: " << received << std::endl;
            }
        } catch (...) {
            // Ignore RX errors
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

static void chat(MyRadio& mySDR) {
    std::queue<std::string> input_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::atomic<bool> exit_flag(false);

    std::thread tx_thread(input_and_transmit,
                          std::ref(input_queue),
                          std::ref(queue_mutex),
                          std::ref(queue_cv),
                          std::ref(exit_flag),
                          std::ref(mySDR));

    std::thread rx_thread(print_message,
                          std::ref(exit_flag),
                          std::ref(mySDR));

    try {
        while (true) {
            std::string user_input;
            if (!std::getline(std::cin, user_input)) {
                break;
            }

            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                input_queue.push(user_input);
            }
            queue_cv.notify_one();

            std::string lower = user_input;
            for (auto& c : lower) c = std::tolower(c);
            if (lower == "q") {
                break;
            }
        }
    } catch (...) {
        // KeyboardInterrupt equivalent
    }

    // Signal quit
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        input_queue.push("q");
    }
    queue_cv.notify_one();
    exit_flag.store(true);

    if (tx_thread.joinable()) tx_thread.join();
    if (rx_thread.joinable()) rx_thread.join();
}

// ============================================================
// Version 1: Full menu (pluto_chat.py equivalent)
// ============================================================

static std::unique_ptr<MyRadio> add_radio_menu_full() {
    std::cout << "Adding a radio " << std::endl;
    std::string ip;
    std::cout << "Enter the ip address: ";
    std::getline(std::cin, ip);
    std::string name;
    std::cout << "Enter your username:";
    std::getline(std::cin, name);
    return std::make_unique<MyRadio>(ip, name);
}

static void show_radio_menu(MyRadio& mySDR) {
    std::cout << "++++++ Radio Parameters ++++++" << std::endl;
    std::cout << mySDR.repr() << std::endl;
}

static void change_radio_menu(MyRadio& mySDR) {
    while (true) {
        std::cout << "++++++++++ Parameter Selection +++++++++" << std::endl;
        std::cout << "1 - Tx Local Oscillator (MHz)" << std::endl;
        std::cout << "2 - Rx Local Oscillator (MHz)" << std::endl;
        std::cout << "3 - Sample Rate (MHz)" << std::endl;
        std::cout << "4 - Tx-Rx Bandwidth (MHz)" << std::endl;
        std::cout << "5 - Tx Gain (dB)" << std::endl;
        std::cout << "6 - Rx Gain Type and Gain (dB) " << std::endl;
        std::cout << "7 - Tx Sample Size 2**N (Enter N)" << std::endl;
        std::cout << "8 - Rx Sample Size 2**N (Enter N)" << std::endl;
        std::cout << "9 - User Name" << std::endl;
        std::cout << "10 - Back" << std::endl;

        std::string line;
        std::cout << "Select a parameter to change: ";
        std::getline(std::cin, line);
        int selection = 0;
        try {
            selection = std::stoi(line);
        } catch (...) {
            continue;
        }

        std::string val_str;
        int val;

        switch (selection) {
            case 1:
                std::cout << "Enter the Tx Frequency in MHz: ";
                std::getline(std::cin, val_str);
                val = std::stoi(val_str);
                mySDR.set_tx_lo(static_cast<long long>(val) * MHZ);
                break;
            case 2:
                std::cout << "Enter the Rx Frequency in MHz: ";
                std::getline(std::cin, val_str);
                val = std::stoi(val_str);
                mySDR.set_rx_lo(static_cast<long long>(val) * MHZ);
                break;
            case 3:
                std::cout << "Enter the Sample Rate in MHz: ";
                std::getline(std::cin, val_str);
                val = std::stoi(val_str);
                mySDR.set_sample_rate(static_cast<long long>(val) * MHZ);
                break;
            case 4:
                std::cout << "Enter the Bandwidth in MHz: ";
                std::getline(std::cin, val_str);
                val = std::stoi(val_str);
                mySDR.set_rx_rf_bandwidth(static_cast<long long>(val) * MHZ);
                mySDR.set_tx_rf_bandwidth(static_cast<long long>(val) * MHZ);
                break;
            case 5:
                std::cout << "Enter the Tx Gain in dB: ";
                std::getline(std::cin, val_str);
                val = std::stoi(val_str);
                mySDR.set_tx_hardwaregain_chan0(static_cast<double>(val));
                break;
            case 6: {
                std::cout << "Enter the Rx Gain Type: \n 1-Manual\n2-Slow Attack \n3-Fast Attack: ";
                std::getline(std::cin, val_str);
                int gain_type = std::stoi(val_str);
                if (gain_type == 1) {
                    std::cout << "Enter Rx Gain in dB: ";
                    std::getline(std::cin, val_str);
                    val = std::stoi(val_str);
                    mySDR.set_gain_control_mode_chan0("manual");
                    mySDR.set_rx_hardwaregain_chan0(static_cast<double>(val));
                } else if (gain_type == 2) {
                    mySDR.set_gain_control_mode_chan0("slow_attack");
                } else if (gain_type == 3) {
                    mySDR.set_gain_control_mode_chan0("fast_attack");
                }
                break;
            }
            case 7:
                std::cout << "Enter the Tx Sample Size: ";
                std::getline(std::cin, val_str);
                val = std::stoi(val_str);
                mySDR.set_tx_length(1 << val);
                break;
            case 8:
                std::cout << "Enter the Rx Sample Size: ";
                std::getline(std::cin, val_str);
                val = std::stoi(val_str);
                mySDR.set_rx_buffer_size(1 << val);
                break;
            case 9:
                std::cout << "Enter the User Name: ";
                std::getline(std::cin, val_str);
                mySDR.set_user_name(val_str);
                break;
            case 10:
                return;
            default:
                break;
        }
    }
}

static void main_menu_full() {
    std::unique_ptr<MyRadio> mySDR;

    while (true) {
        std::cout << "*** PlutoChat ***" << std::endl;
        std::cout << "1. Add a radio" << std::endl;
        std::cout << "2. Show the radio parameters" << std::endl;
        std::cout << "3. Change Radio Parameters" << std::endl;
        std::cout << "4. Chat" << std::endl;
        std::cout << "5. Quit" << std::endl;

        std::string line;
        std::getline(std::cin, line);
        int choice = 0;
        try {
            choice = std::stoi(line);
        } catch (...) {
            continue;
        }

        switch (choice) {
            case 1:
                try {
                    mySDR = add_radio_menu_full();
                } catch (const std::exception& e) {
                    std::cerr << "Error adding radio: " << e.what() << std::endl;
                }
                break;
            case 2:
                if (mySDR) show_radio_menu(*mySDR);
                else std::cerr << "No radio added yet." << std::endl;
                break;
            case 3:
                if (mySDR) change_radio_menu(*mySDR);
                else std::cerr << "No radio added yet." << std::endl;
                break;
            case 4:
                if (mySDR) chat(*mySDR);
                else std::cerr << "No radio added yet." << std::endl;
                break;
            case 5:
                std::cout << "Quitting Program..." << std::endl;
                return;
            default:
                break;
        }
    }
}

// ============================================================
// Version 2: Simplified menu (pluto_chat_wo_setting.py equivalent)
// ============================================================

// SDR default parameters
static const long long TX_LO_FREQUENCY = 1000LL * MHZ;
static const long long RX_LO_FREQUENCY = 900LL * MHZ;
static const long long SAMPLING_RATE = 10LL * MHZ;
static const long long BANDWIDTH = 10LL * MHZ;
static const double TX_GAIN = 0.0;
static const std::string RX_GAIN_TYPE = "slow_attack";
static const int TX_SAMPLE_SIZE = (1 << 18);  // 2^18
static const int RX_SAMPLE_SIZE = (1 << 16);  // 2^16

static std::unique_ptr<MyRadio> add_radio_menu_simple() {
    std::cout << "Adding a radio " << std::endl;
    std::string ip;
    std::cout << "Enter the ip address: ";
    std::getline(std::cin, ip);
    std::string name;
    std::cout << "Enter your username:";
    std::getline(std::cin, name);

    auto mySDR = std::make_unique<MyRadio>(ip, name);

    mySDR->set_tx_lo(TX_LO_FREQUENCY);
    mySDR->set_rx_lo(RX_LO_FREQUENCY);
    mySDR->set_sample_rate(SAMPLING_RATE);
    mySDR->set_rx_rf_bandwidth(BANDWIDTH);
    mySDR->set_tx_rf_bandwidth(BANDWIDTH);
    mySDR->set_tx_hardwaregain_chan0(TX_GAIN);
    mySDR->set_gain_control_mode_chan0(RX_GAIN_TYPE);
    mySDR->set_rx_buffer_size(RX_SAMPLE_SIZE);
    mySDR->set_tx_length(TX_SAMPLE_SIZE);

    return mySDR;
}

static void main_menu_simple() {
    std::unique_ptr<MyRadio> mySDR;

    while (true) {
        std::cout << "*** PlutoChat ***" << std::endl;
        std::cout << "1. Add a radio" << std::endl;
        std::cout << "2. Show the radio parameters" << std::endl;
        std::cout << "3. Chat" << std::endl;
        std::cout << "4. Quit" << std::endl;

        std::string line;
        std::getline(std::cin, line);
        int choice = 0;
        try {
            choice = std::stoi(line);
        } catch (...) {
            continue;
        }

        switch (choice) {
            case 1:
                try {
                    mySDR = add_radio_menu_simple();
                } catch (const std::exception& e) {
                    std::cerr << "Error adding radio: " << e.what() << std::endl;
                }
                break;
            case 2:
                if (mySDR) show_radio_menu(*mySDR);
                else std::cerr << "No radio added yet." << std::endl;
                break;
            case 3:
                if (mySDR) chat(*mySDR);
                else std::cerr << "No radio added yet." << std::endl;
                break;
            case 4:
                std::cout << "Quitting Program..." << std::endl;
                return;
            default:
                break;
        }
    }
}

// ============================================================
// Entry point
// ============================================================

int main(int argc, char* argv[]) {
    std::cout << "Welcome to PlutoChat!" << std::endl;

    bool full_mode = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--full" || arg == "-f") {
            full_mode = true;
        }
    }

    try {
        if (full_mode) {
            main_menu_full();
        } else {
            main_menu_simple();
        }
    } catch (...) {
        std::cout << "\nApplication terminated by user. Goodbye!" << std::endl;
    }

    return 0;
}
