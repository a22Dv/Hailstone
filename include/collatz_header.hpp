#ifndef COLLATZ_HPP_
#define COLLATZ_HPP_

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstdint>
#include <memory>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <csignal>
#include <atomic>
#include <variant>
#include <random>
#include <functional>
#include <any>
#include <unordered_set>

class Utilities;
void handleSigterm(int sigNum);

class Application {
private:
    const std::string configPath = "";
    std::unordered_map<std::string, std::string> config = {};
    std::unique_ptr<Utilities> utilInstance = nullptr;
    std::vector<uint64_t> getSequence(uint32_t n);

    
public:
    Application(std::unique_ptr<Utilities> utils);
    void start();
};
using ConfigContainer = std::variant<std::string, uint32_t, std::pair<uint32_t, uint32_t>, std::vector<uint8_t>, std::pair<std::vector<uint8_t>, std::vector<uint8_t>>, bool>;
class Utilities {
private:
    const char messageStart = 'm';
    const char resultStart = 'r';
    const char endDelimiter = '/';
    const char dividerNum = '.';
    const char dividerSeq = '|';
    uint32_t getNumber(const std::string &str);
    std::vector<uint8_t> getColor(const std::string &str);
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> getGradientColor(const std::string &str);
    std::pair<uint32_t, uint32_t> getImageSize(const std::string &str);
    bool getBool(const std::string &str);

public:
    std::string wrapMessage(const std::string &message);
    std::vector<uint32_t> getValuesToEvaluate(const std::string &userInput, const std::unordered_map<std::string, std::string> &config);
    std::string wrapSequences(const std::vector<std::vector<uint64_t>> &sequences);
    std::vector<std::string> splitByChar(char c, const std::string &str); 
    std::unordered_map<std::string, std::string> getConfig();
    ConfigContainer extractFromConfig(const std::string &field, const std::unordered_map<std::string, std::string> &config);
};

#endif