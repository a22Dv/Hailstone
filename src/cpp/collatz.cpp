#include "compute_collatz_header.hpp"

/// @brief Main program process. Handles stdin and stdout for the program.
void Collatz::main_process() {
    while (true) {
        std::string stdinInput;
        std::getline(std::cin, stdinInput);

        // Holds the range of numbers. Both ends inclusive.
        std::vector<std::string> procInput = splitString(stdinInput, '|');
        std::pair<int32_t, int32_t> args = {std::stoi(procInput[0]), std::stoi(procInput[1])};
        std::vector<std::vector<int64_t>> sequences = {};
        for (int32_t i = args.first; i <= args.second; ++i) {
            sequences.push_back(get_sequence(i));
        }
        std::stringstream ss;
        const size_t seqsSize = sequences.size();
        for (size_t i = 0; i < seqsSize; ++i) {
            std::vector<int64_t> sequence = sequences[i];
            const size_t seqSize = sequence.size();
            for (size_t j = 0; j < seqSize; ++j) {
                int64_t integer = sequence[j];
                ss << integer;
                if (j != seqSize - 1) {
                    ss << '|';
                }
                
            }
            if (i != seqsSize - 1) {
                ss << "!";
            }
        }
        std::cout << ss.str() << std::endl;
    }
}

/// @brief Returns the sequence of hailstone numbers from a seed number.
/// @param seed The number to start with.
/// @return An `std::vector<int64_t>` that holds a sequence of numbers.
std::vector<int64_t> Collatz::get_sequence(int32_t seed) {
    std::vector<int64_t> sequence = {};
    int32_t current = seed;
    sequence.push_back(seed);
    while (current != 1) {
        if (current % 2 == 0) {
            current /= 2;
        } else {
            current = 3 * current + 1;
        }
        sequence.push_back(current);
    }
    return sequence;
}

/// @brief A utility function to split strings.
/// @param str The string to split.
/// @param delimiter A delimiter to guide where to split.
/// @return An `std::vector<std::string>` that holds the split strings.
std::vector<std::string> Collatz::splitString(std::string str, char delimiter) {
    std::vector<std::string> strings;
    const size_t strLen = str.length();
    std::stringstream ss;
    for (size_t i = 0; i < strLen; ++i) {
        char c = str[i];
         if (c != '|') {
            ss << c;
        } else {
            strings.push_back(ss.str());
            ss.str("");
        }
        if (i == strLen - 1) {
            strings.push_back(ss.str());
        }

    }
    return strings;
}
