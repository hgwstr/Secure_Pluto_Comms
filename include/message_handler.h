#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <string>

// Process received message frames and reconstruct the complete message
std::string message_handler(const std::string& received_message, bool display_outputs);

#endif
