#include "message_handler.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

static const std::string FRAME_FILLER =
    "00ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890";

// Split string by delimiter
static std::vector<std::string> split_string(const std::string& str, const std::string& delim) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t pos;
    while ((pos = str.find(delim, start)) != std::string::npos) {
        result.push_back(str.substr(start, pos - start));
        start = pos + delim.size();
    }
    result.push_back(str.substr(start));
    return result;
}

std::string message_handler(const std::string& received_message, bool display_outputs) {
    std::vector<std::string> split_array = split_string(received_message, "0X");

    std::vector<int> id_array;
    std::vector<std::string> frame_array;
    // id_and_substrings stored as pairs: [id, text, id, text, ...]
    std::vector<std::string> id_and_substrings;

    bool equal_flag = false;

    for (int idx = 0; idx < static_cast<int>(split_array.size()); ++idx) {
        if ((idx <= static_cast<int>(split_array.size()) - 2) &&
            !split_array[idx].empty()) {
            if (split_array[idx].size() >= 2 &&
                split_array[idx + 1].size() >= 2 &&
                split_array[idx].substr(0, 2) == split_array[idx + 1].substr(0, 2) &&
                FRAME_FILLER.find(split_array[idx]) == std::string::npos) {
                frame_array.push_back(split_array[idx]);
            }
        }
    }

    for (int idx = 0; idx < static_cast<int>(frame_array.size()); ++idx) {
        if (idx != static_cast<int>(frame_array.size()) - 1) {
            std::string current_substring = frame_array[idx];
            std::string next_substring = frame_array[idx + 1];
            int current_id, next_id;
            try {
                current_id = std::stoi("0x" + current_substring.substr(0, 2), nullptr, 16);
            } catch (...) {
                continue;
            }
            try {
                next_id = std::stoi("0x" + next_substring.substr(0, 2), nullptr, 16);
            } catch (...) {
                continue;
            }

            if (!equal_flag) {
                if (current_id != next_id) {
                    id_array.push_back(current_id);
                    std::string trimmed = current_substring.substr(2);
                    id_and_substrings.push_back(std::to_string(current_id));
                    id_and_substrings.push_back(trimmed);
                } else {
                    equal_flag = true;
                }
            } else {
                if (idx > 0) {
                    std::string previous_substring = frame_array[idx - 1];
                    int previous_id;
                    try {
                        previous_id = std::stoi("0x" + previous_substring.substr(0, 2), nullptr, 16);
                    } catch (...) {
                        continue;
                    }
                    id_array.push_back(previous_id);
                    std::string trimmed = previous_substring.substr(2);
                    id_and_substrings.push_back(std::to_string(previous_id));
                    id_and_substrings.push_back(trimmed);
                }
                equal_flag = false;
            }
        } else {
            if (!equal_flag) {
                std::string current_substring = frame_array[idx];
                int current_id;
                try {
                    current_id = std::stoi("0x" + current_substring.substr(0, 2), nullptr, 16);
                } catch (...) {
                    continue;
                }
                id_array.push_back(current_id);
                std::string trimmed = current_substring.substr(2);
                id_and_substrings.push_back(std::to_string(current_id));
                id_and_substrings.push_back(trimmed);
            } else {
                if (idx > 0) {
                    std::string current_substring = frame_array[idx - 1];
                    int current_id;
                    try {
                        current_id = std::stoi("0x" + current_substring.substr(0, 2), nullptr, 16);
                    } catch (...) {
                        continue;
                    }
                    id_array.push_back(current_id);
                    std::string trimmed = current_substring.substr(2);
                    id_and_substrings.push_back(std::to_string(current_id));
                    id_and_substrings.push_back(trimmed);
                }
            }
        }
    }

    if (id_array.empty()) {
        return "";
    }

    int max_id = *std::max_element(id_array.begin(), id_array.end());
    int id_and_substrings_len = static_cast<int>(id_and_substrings.size());
    int storage_matrix_columns = max_id + 1;
    int storage_matrix_rows = static_cast<int>(
        std::ceil(static_cast<double>(id_and_substrings_len) / 2.0 /
                  storage_matrix_columns)) + 1;

    // 2D storage matrix (rows x cols) initialized to "0"
    std::vector<std::vector<std::string>> message_storage_matrix(
        storage_matrix_rows, std::vector<std::string>(storage_matrix_columns, "0"));

    int row_counter = 0;
    for (int i = 0; i < id_and_substrings_len - 1; i += 2) {
        int prefix = std::stoi(id_and_substrings[i]);
        int next_prefix;
        if (i + 2 < id_and_substrings_len) {
            next_prefix = std::stoi(id_and_substrings[i + 2]);
        } else {
            next_prefix = prefix;
        }
        std::string text_part = id_and_substrings[i + 1];

        if (prefix <= max_id) {
            message_storage_matrix[row_counter][prefix] = text_part;
            if (prefix == max_id || std::abs(next_prefix - prefix) > 1) {
                row_counter++;
                if (row_counter >= storage_matrix_rows) break;
            }
        }
    }

    // Flip matrix upside down
    std::vector<std::vector<std::string>> flipped(message_storage_matrix.rbegin(),
                                                   message_storage_matrix.rend());

    // Flatten to row vector (hstack of flipped)
    std::vector<std::string> row_text_vector;
    for (const auto& row : flipped) {
        for (const auto& cell : row) {
            row_text_vector.push_back(cell);
        }
    }

    // Reconstruct final message
    std::vector<std::string> final_message(max_id + 1, "");
    int message_part_counter = 0;

    for (const auto& partition : row_text_vector) {
        if (message_part_counter > max_id) break;
        if (partition == "0") continue;
        if (partition.size() >= 5 &&
            FRAME_FILLER.find(partition.substr(0, 5)) != std::string::npos) {
            continue;
        }

        // Check if already in final_message
        bool already_exists = false;
        for (int j = 0; j < message_part_counter; ++j) {
            if (final_message[j] == partition) {
                already_exists = true;
                break;
            }
        }
        if (already_exists) continue;

        final_message[message_part_counter] = partition;
        message_part_counter++;
    }

    if (display_outputs) {
        std::cout << "splitArray:" << std::endl;
        for (const auto& s : split_array) std::cout << "  " << s << std::endl;
        std::cout << "frameArray:" << std::endl;
        for (const auto& s : frame_array) std::cout << "  " << s << std::endl;
    }

    std::string message_output;
    for (const auto& part : final_message) {
        message_output += part;
    }
    return message_output;
}
