#ifndef OPERATION_TX_H
#define OPERATION_TX_H

#include "my_radio.h"
#include <string>

// Perform full TX operation: frame generation -> signal generation -> transmit
void operation_TX(MyRadio& my_SDR, const std::string& msg, bool plotGraphs, bool info);

#endif
