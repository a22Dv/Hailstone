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

/// @brief Floating Point 32-bit precision.
/// @typedef F32
using F32 = float;

/// @brief RGBA color space, values held in their respective order.
/// @details Values stored as [Red, Green, Blue, Alpha]. [0-255].
/// @typedef RGBA
using RGBA = std::array<uint8_t, 4>;

/// @brief HSVA color space, values held in their respective order. 
/// @details Values stored as: [Hue, Saturation, Value, Alpha]. All values normalized from [0.0 - 1.0].
/// @typedef HSVA
using HSVA = std::array<F32, 4>;

/// @brief Gradient. Holds two RGBA values for the start and end colors.
/// @typedef Gradient
using Gradient = std::pair<RGBA, RGBA>;

/// @brief Image Dimensions. Value is in px, [Width, Height].
/// @typedef ImageDimensions
using ImageDimensions = std::pair<uint32_t, uint32_t>;

/// @brief Range. Values stored as [start, end].
/// @typedef Range
using Range = std::pair<uint32_t, uint32_t>;

/// @brief Arithmetic. Constrains a type to be of arithmetic type. (e.g. int, float, double)
template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

/// @brief A class that holds utilities for vectors.
/// @details  Although implemented in the std:: library, this code serves as an introduction
/// point for the developer when it comes to template programming.
class VectorUtilities {
public:
    /// @brief Gets the maximum arithmetic value in a given vector.
    /// @tparam T Any arithmetic type.
    /// @param values A vector with arithmetic values.
    /// @return The maximum value in `values`.
    template <Arithmetic T>
    static T getMax(const std::vector<T>& values) {
        if (values.empty()) {
            throw std::invalid_argument("Cannot find max in empty vector.");
        }
        T maxVal = *values.begin();
        for (const T &v : values) {
            if (v > maxVal) {
                maxVal = v;
            }
        }
        return maxVal;
    };

    /// @brief Gets the minimum arithmetic value in a given vector.
    /// @tparam T Any arithmetic type.
    /// @param values A vector with arithmetic values.
    /// @return The minimum value in `values`.
    template <Arithmetic T>
    static T getMin(const std::vector<T>& values) {
        if (values.empty()) {
            throw std::invalid_argument("Cannot find min in empty vector.");
        }
        T minVal = values[0];
        for (const T &v : values) {
            if (v < minVal) {
                minVal = v;
            }
        }
        return minVal;
    };
};

class StringUtilities {
public:
    static std::vector<std::string> split(const std::string &str, const std::string &delimiter);
    static std::string strip(const std::string &str);
};

class MathUtilities {
public:
    static F32 getRadians(F32 degrees);
};

class ColorUtilities {
public:
    static RGBA HSVAToRGBA(const HSVA &hsva);
    static HSVA RGBAToHSVA(const RGBA &rgba);
    static F32 getHue(const std::unordered_map<std::string, std::vector<F32>> &coordinates, const std::vector<uint32_t> &frequencyMap, bool isFrequencyBased);
    static F32 getSaturation(const std::unordered_map<std::string, std::vector<F32>> &coordinates, const std::vector<uint32_t> &frequencyMap, bool isFrequencyBased);
    static F32 getValue(const std::unordered_map<std::string, std::vector<F32>> &coordinates, const std::vector<uint32_t> &frequencyMap, bool isFrequencyBased);
};

class ConfigUtilities {
public:
    static std::unordered_map<std::string, std::string> getConfig(const fs::path &configPath);
    static uint32_t getValue(const std::string &strValue);
    static ImageDimensions getDimensions(const std::string &imageSize);
    static F32 getFloatValue(const std::string &strFloat);  
    static Gradient getGradient(const std::string &gradientHexes);
    static RGBA getRGBA(const std::string &rgbaHex);
    static fs::path getExecutablePath();
};

class SubprocessUtilities {
public:
    static inline const std::vector<std::string> coordinateParameters = {"x1", "x2", "x3", "x4", "y1", "y2", "y3", "y4"};
    static Range getRange(const std::string &rangeStr);
    static std::string assembleValues(
        const std::unordered_map<std::string, std::vector<F32>> &coordinates,
        const std::unordered_map<std::string, std::vector<uint8_t>> &style
    );
    static std::unordered_map<std::string, uint32_t> getFrequencyMap(const std::vector<std::vector<uint64_t>> &sequences);
    static std::string getStrRepr(const uint64_t a, const uint64_t b);
    static std::array<uint64_t, 2> getIntsFromRepr(const std::string &repr);
    static std::unordered_map<std::string, uint32_t> getSegmentPosition(const std::unordered_map<std::string, uint32_t> &frequencyMap);
    static RGBA getRGBASegmentValue(HSVA gradientStart, HSVA gradientEnd, uint32_t position, uint32_t nPositions);
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
    std::unique_ptr<IPC> ipc = nullptr;
    std::unordered_map<std::string, std::string> config;
public:
    Subprocess(std::unique_ptr<IPC> ipc);
    void start();
    std::vector<uint32_t> getValues(const Range &range);
    std::vector<std::vector<uint64_t>> getSequences(const std::vector<uint32_t> &values);
    std::vector<uint64_t> getSequence(uint32_t n);
    std::unordered_map<std::string, std::vector<F32>> getCoordinates(const std::vector<std::vector<uint64_t>> &sequences);
    std::unordered_map<std::string, std::vector<uint8_t>> getStyles(const std::vector<std::vector<uint64_t>> &sequences, const std::unordered_map<std::string, uint32_t> &frequencyMap);
    void quit();
};
#endif