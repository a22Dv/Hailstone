#include "collatz_header.hpp"

int main(int argc, char* argv[]) {

    // Can be stack-allocated, which is easier. This is just a practice for using pointers.
    std::unique_ptr<Utilities> utils = std::make_unique<Utilities>();
    std::unique_ptr<Application> app = std::make_unique<Application>(std::move(utils));
    app->start();
    return 0;
}

/// @brief Constructor for `Application`.
/// @param utils A `Utilities` instance to use.
Application::Application(std::unique_ptr<Utilities> utils) : utilInstance(std::move(utils)) {};
void Application::start() {

    // Process successfully started.
    std::cout << utilInstance->wrapMessage("200");
    while (true) {
        std::string userInput = "";
        std::getline(std::cin, userInput);
    }
}

/// @brief Returns a vector holding the collatz sequence for a number `n`.
/// @param n The value to get the sequence of.
/// @return An `std::vector<uint64_t>` holding the collatz sequence for `n`.
std::vector<uint64_t> Application::getSequence(uint32_t n) {
    std::vector<uint64_t> sequence = {n};
    uint64_t currentNumber = static_cast<uint64_t>(n);
    while (currentNumber != 1) {
        if (currentNumber & 1) {
            currentNumber = 3 * currentNumber + 1;
        } else {
            currentNumber /= 2;
        }
        sequence.push_back(currentNumber);
    }
    return sequence;
}

/// @brief Returns split strings by a specified `char` delimeter. This function will ignore empty delimited sequences e.g. `",,,"`.
/// @param c Delimeter.
/// @param str The string to be split.
/// @return Resulting splits as an `std::vector<std::string>`.
std::vector<std::string> Utilities::splitByChar(char c, const std::string &str) {
    std::vector<std::string> splitStrings = {};
    std::stringstream ss;
    for (char strC : str) {
        if (strC != c) {
            ss << strC;
        } else {
            splitStrings.push_back(ss.str());
            ss.str("");
        }
    }
    std::string remainder = ss.str();
    if (remainder.length() != 0) {
        splitStrings.push_back(remainder);
    }
    return splitStrings;
}

std::string Utilities::wrapMessage(const std::string &message) {
    std::stringstream ss;
    ss << messageStart << message << endDelimiter << messageStart;
    return ss.str();
}

std::string Utilities::wrapSequences(const std::vector<std::vector<uint64_t>> &sequences) {
    std::stringstream ss;
    ss << resultStart;
    for (std::vector<uint64_t> seq : sequences) {
        for (uint64_t n : seq) {
            ss << n << dividerNum;
        }
        ss << dividerSeq;
    }
    ss << endDelimiter << resultStart;
    return ss.str();
}