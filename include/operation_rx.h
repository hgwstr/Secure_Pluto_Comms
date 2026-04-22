#ifndef OPERATION_RX_H
#define OPERATION_RX_H

#include "my_radio.h"
#include <string>

// Perform full RX operation: receive -> process -> decode message
// Returns decoded message or empty string on failure
std::string operation_RX(MyRadio& my_SDR, bool plot_graphs);

#endif
