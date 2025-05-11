#ifndef COLLATZ_HPP_
#define COLLATZ_HPP_

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstdint>
#include <memory>

class Application {
private:
    std::unique_ptr<Utilities> utilInstance = nullptr;
    std::vector<uint64_t> getSequence(uint32_t n);
public:
    Application(const std::unique_ptr<Utilities> utils);
    void start();
};

class Utilities {
private:
    const char messageStart = 'm';
    const char resultStart = 'r';
    const char endDelimiter = '/';
    const char dividerNum = '.';
    const char dividerSeq = '|';
public:
    std::string wrapMessage(const std::string &message);
    std::string wrapSequences(const std::vector<std::vector<uint64_t>> &sequences);
    std::vector<std::string> splitByChar(char c, const std::string &str); 
};
#endif