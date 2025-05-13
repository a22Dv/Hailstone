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

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

using RGBA = std::array<uint8_t, 4>;
using Gradient = std::pair<RGBA, RGBA>;
using ImageDimensions = std::pair<uint32_t, uint32_t>;
using Range = std::pair<uint32_t, uint32_t>;
using F32 = float;



class Utilities {
public:
    static std::vector<std::string> split(const std::string &str, const std::string &delimiter);
    static std::string strip(const std::string &str);
    RGBA getRGBA(const std::string &rgbaHex);
    Gradient getGradient(const std::string &gradientHexes);
    uint32_t getValue(const std::string &strValue);
    ImageDimensions getDimensions(const std::string &imageSize);
    F32 getFloatValue(const std::string &strFloat);  
    std::unordered_map<std::string, std::string> getConfig(const fs::path &configPath);
    fs::path getExecutablePath();
};

class IO {
public:
    Range getRange();
};


class Subprocess {
private:
    std::unique_ptr<Utilities> utilities = nullptr;
    std::unique_ptr<IO> io = nullptr;
    std::unordered_map<std::string, std::string> config;
    const std::string processFinished = "/1";
    const std::string sendData = "/2";
    
public:
    Subprocess(std::unique_ptr<Utilities> utilities, std::unique_ptr<IO> io);
    void start();
    std::vector<uint32_t> getValues(const Range &range);
    std::vector<std::vector<uint32_t>> getSequences(const std::vector<uint32_t> &values);
    std::vector<uint32_t> getSequence(uint32_t n);
    void quit();
};
#endif