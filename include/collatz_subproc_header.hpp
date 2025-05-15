#ifndef COLLATZ_SUBPROC_HEADER_HPP_
#define COLLATZ_SUBPROC_HEADER_HPP_

#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <variant>
#include <cstdint>
#include <array>
#include <filesystem>
#include <stdexcept>
#include <cctype>
#include <fstream>
#include <cstring>
#include <random>
#include <cmath>
#include <numbers>
#include <concepts>



#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

using RGBA = std::array<uint8_t, 4>;
using HSV = std::array<float, 3>;
using Gradient = std::pair<RGBA, RGBA>;
using ImageDimensions = std::pair<uint32_t, uint32_t>;
using Range = std::pair<uint32_t, uint32_t>;

using F32 = float;

class Utilities {
public:
    Utilities();
    static std::vector<std::string> split(const std::string &str, const std::string &delimiter);
    static std::string strip(const std::string &str);
    RGBA getRGBA(const std::string &rgbaHex);
    HSV getHSV(const RGBA &rgba);
    Gradient getGradient(const std::string &gradientHexes);
    uint32_t getValue(const std::string &strValue);
    ImageDimensions getDimensions(const std::string &imageSize);
    F32 getFloatValue(const std::string &strFloat);  
    std::unordered_map<std::string, std::string> getConfig(const fs::path &configPath);
    fs::path getExecutablePath();
    Range getRange(const std::string &rangeStr);
    std::string assembleValues(
        const std::unordered_map<std::string, std::vector<F32>> &coordinates,
        const std::unordered_map<std::string, std::vector<uint8_t>> &style
    );
    F32 getRadians(F32 degrees);
};

class IPC {
private:
    bool text = false;
public:
    IPC(bool text);
    const std::unordered_map<std::string, std::string> codes = {
        {"send", "\n"},
        {"processingFinished", "/1"},
        {"sendData", "/2"},
        {"terminate", "/-1"},
        {"failureToReceive", "/-2"},
    };
    void send(const std::string &message, bool stdOut = true);
    std::string receive();
};

class Subprocess {
private:
    std::unique_ptr<Utilities> utilities = nullptr;
    std::unique_ptr<IPC> ipc = nullptr;
    std::unordered_map<std::string, std::string> config;
public:
    Subprocess(std::unique_ptr<Utilities> utilities, std::unique_ptr<IPC> ipc);
    void start();
    std::vector<uint32_t> getValues(const Range &range);
    std::vector<std::vector<uint64_t>> getSequences(const std::vector<uint32_t> &values);
    std::vector<uint64_t> getSequence(uint32_t n);
    std::unordered_map<std::string, std::vector<F32>> getCoordinates(const std::vector<std::vector<uint64_t>> &sequences);
    std::unordered_map<std::string, std::vector<uint8_t>> getStyles(const std::vector<std::vector<uint64_t>> &sequences);
    void quit();
};
#endif