#ifndef COLLATZ_SUBPROC_HEADER_HPP_
#define COLLATZ_SUBPROC_HEADER_HPP_

// std:: includes.
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
#include <algorithm>

// Windows-specific, for getting the executable location at runtime.
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// For .yaml config file parsing.
#include "yaml-cpp/yaml.h"


namespace fs = std::filesystem;

/// @brief Floating point 32-bit precision.
using F32 = float;

/// @brief RGBA color space, values held in their respective order. [0-255]
/// @details Values stored as [Red, Green, Blue, Alpha]. [0-255].
using RGBA = std::array<uint8_t, 4>;

/// @brief HSVA color space, values held in their respective order. Normalized [0.0-1.0]
/// @details Values stored as: [Hue, Saturation, Value, Alpha]. All values normalized from [0.0 - 1.0].
using HSVA = std::array<F32, 4>;

/// @brief Gradient. Holds two RGBA values for the start and end colors.
/// @details Stored as [Start, End]
using Gradient = std::pair<RGBA, RGBA>;

/// @brief Image Dimensions. Value is in px, [Width, Height].
using ImageDimensions = std::pair<uint32_t, uint32_t>;

/// @brief Range. Values stored as [start, end].
using Range = std::pair<uint32_t, uint32_t>;

/// @brief Arithmetic. Constrains a type to be of arithmetic type. (e.g. `int`, `float`, `double`)
template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

/// @brief A class that holds utilities for vectors.
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

/// @brief A class that holds utilities for string manipulation.
class StringUtilities {
public:

    /// @brief Splits a string according to a substring delimiter.
    /// @param str The string to be split.
    /// @param delimiter The delimiter being used to split the string.
    /// @return A vector holding the split substrings.
    static std::vector<std::string> split(const std::string &str, const std::string &delimiter);
    static std::string strip(const std::string &str);
};

/// @brief A class that holds utilities for math-related utilities.
class MathUtilities {
public:
    /// @brief Converts degrees to radians.
    /// @param degrees Angle in degrees.
    /// @return `F32` value in Radians.
    static F32 getRadians(F32 degrees);
};

/// @brief A class that holds utilities for colors.
class ColorUtilities {
public:
    /// @brief Converts `HSVA` to `RGBA`.
    /// @param hsva Color value in `HSVA` (Normalized [0.0-1.0])
    /// @return The same color value in `RGBA`.
    static RGBA HSVAToRGBA(const HSVA &hsva);

    /// @brief Converts `RGBA` to `HSVA`.
    /// @param rgba Color value in `RGBA`.
    /// @return The same color value in `HSVA`.
    static HSVA RGBAToHSVA(const RGBA &rgba);

    /// @brief Returns the `RGBA` value for a given segment given its rank and a number of ranks specified. 
    /// @details Within the total number of ranks, a position closer to 0 means the given color is closer to the starting gradient, 
    /// closer to the end means closer to the end of the gradient.
    /// @param gradientStart The color of the start of the gradient in `HSVA`.
    /// @param gradientEnd The color of the end of the gradien in `HSVA`.
    /// @param position The segment's position in a ranking.
    /// @param nPositions Number of ranks.
    /// @return The RGBA value associated with that rank within the gradient.
    static RGBA getRGBASegmentValue(HSVA gradientStart, HSVA gradientEnd, uint32_t position, uint32_t nPositions);
};

/// @brief A class that holds utilities for getting values with a given config file.
class ConfigUtilities {
public:

    /// @brief Extracts the configuration file's information as strings in key-value pairs.
    /// @param configPath Path to the `.yaml` configuration file.
    /// @return A map to the configuration file's information in key-value pairs.
    static std::unordered_map<std::string, std::string> getConfig(const fs::path &configPath);

    /// @brief Returns a `uint32_t` value from a string. Specifically for the .yaml config file. (Base-10)
    /// @param strValue The string that contains the value.
    /// @return The `uint32_t` representation of the value 
    static uint32_t getValue(const std::string &strValue);

    /// @brief Returns an `ImageDimensions` object holding the image dimensions associated with the configuration file.
    /// @param imageSize The string containing the image size / dimensions.
    /// @return An `ImageDimensions` containing the image dimensions.
    static ImageDimensions getDimensions(const std::string &imageSize);

    /// @brief Gets a float value from a string.
    /// @param strFloat The string containing the float.
    /// @return The `F32` representation of the value in the string.
    static F32 getFloatValue(const std::string &strFloat);  
    
    /// @brief Gets a `Gradient` object from the string containing the gradient hexes from the configuration file.
    /// @param gradientHexes A string containing two Hexes that hold the color for both ends of the gradient.
    /// @return A `Gradient` holding the gradient values.
    static Gradient getGradient(const std::string &gradientHexes);
    
    /// @brief Gets the RGBA value from a string holding its hex representation.
    /// @param rgbaHex A string containing the hex representation of an RGBA value.
    /// @return An `RGBA` color value.
    static RGBA getRGBA(const std::string &rgbaHex);
    
    /// @brief Gets the path of the running executable.
    /// @return Returns the path of the running executable.
    static fs::path getExecutablePath();
};

/// @brief A class holding the main utilities for the main `Subprocess` class.
class SubprocessUtilities {
public:

    /// @brief How coordinates are arranged in any given segment.
    static inline const std::vector<std::string> coordinateParameters = {"x1", "x2", "x3", "x4", "y1", "y2", "y3", "y4"};

    /// @brief Returns the range value in a given string.
    /// @param rangeStr The string holding the range value.
    /// @return A `Range` containing the start and end values of a given range.
    static Range getRange(const std::string &rangeStr);

    /// @brief Serializes the given information into a single string for IPC.
    /// @param coordinates the coordinates of the image.
    /// @param style The style, colors, etc. of the image.
    /// @return A single string containing the serialized information.
    static std::string assembleValues(
        const std::unordered_map<std::string, std::vector<F32>> &coordinates,
        const std::unordered_map<std::string, std::vector<uint8_t>> &style
    );

    /// @brief Gets a frequency map with the key being a string representation of the two points making up a segment.
    /// @param sequences The sequences to be evaluated.
    /// @return A frequency map holding the frequency of each segment across all sequences.
    static std::unordered_map<std::string, uint32_t> getFrequencyMap(const std::vector<std::vector<uint64_t>> &sequences);

    /// @brief A serialized string representation of two values.
    /// @param a Starting value.
    /// @param b Ending value.
    /// @return A serialized representation of both values.
    static std::string getStrRepr(const uint64_t a, const uint64_t b);

    /// @brief Gets the `uint64_t` values from the string representation.Works in tandem with `getStrRepr`.
    /// @param repr A serialized string.
    /// @return The values within the serialized string.
    static std::array<uint64_t, 2> getIntsFromRepr(const std::string &repr);

    /// @brief Gets a mapping from a segment to their ranking in the frequency map. Values closer to 0 represents the highest frequency.
    /// @param frequencyMap The frequency map for the mapping to be based on.
    /// @param sequences The sequences to base the segments on to decode the frequency map.
    /// @return A mapping of the string representation of a segment to their ranking in terms of frequency.
    static std::unordered_map<std::string, uint32_t> getSegmentsRankings(
        const std::unordered_map<std::string, uint32_t> &frequencyMap, 
        const std::vector<std::vector<uint64_t>> &sequences
    );
   
};

/// @brief Class that holds methods for IPC between the main Python process and this C++ subprocess.
class IPC {
private:

    /// @brief Is IPC mode is in text (ASCII).
    bool text = false;
public:

    /// @brief Default constructor.
    /// @param text Bool to determine if IPC channels will be in text (ASCII) or binary.
    IPC(bool text);

    /// @brief IPC codes to use to communicate to the Python parent process.
    const std::unordered_map<std::string, std::string> codes = {
        {"send", "\n"},
        {"processingFinished", "/1"},
        {"sendData", "/2"},
        {"terminate", "/-1"},
        {"failureToReceive", "/-2"},
    };

    /// @brief Sends a message to the parent process.
    /// @param message A string that holds either raw bytes or a string. The data to be sent.
    /// @param stdOut Determines whether message is sent via stdout or stderr.
    void send(const std::string &message, bool stdOut = true);
 
    /// @brief Receive a message from the parent process. Is blocking.
    std::string receive();
};

/// @brief Main class for this C++ subprocess.
class Subprocess {
private:

    /// @brief `std::unique_ptr` to an IPC object for communication to the parent process. 
    std::unique_ptr<IPC> ipc = nullptr;

    /// @brief Holds the configuration information as string key-value pairs.
    std::unordered_map<std::string, std::string> config;
public:

    /// @brief Default constructor
    /// @param ipc A pointer to an IPC instance.
    Subprocess(std::unique_ptr<IPC> ipc);

    /// @brief Main entry point. Starts the subprocess.
    void start();

    /// @brief Gets the values to be evaluated based on configuration and range.
    /// @param range The range to evaluate.
    /// @return A vector of values to be evaluated based on configuration and range.
    std::vector<uint32_t> getValues(const Range &range);

    /// @brief Gives the hailstone sequences associated with the values passed in.
    /// @param values The values to evaluate.
    /// @return A vector containing a vector of values representing the hailstone sequences for a given n.
    std::vector<std::vector<uint64_t>> getSequences(const std::vector<uint32_t> &values);

    /// @brief Gets the hailstone sequence for a given n.
    /// @param n The value to evaluate.
    /// @return A vector containing the hailstone sequence for n.
    std::vector<uint64_t> getSequence(uint32_t n);

    /// @brief Returns the coordinates based on the sequence and configuration that serve as vertices in the final image for all sequences.
    /// @param sequences The hailstone sequences to be evaluated.
    /// @return A map of coordinates with each ith index of a vector being a value for a coordinate of the ith segment.
    std::unordered_map<std::string, std::vector<F32>> getCoordinates(const std::vector<std::vector<uint64_t>> &sequences);

    /// @brief Returns the `RGBA` color values for each segment depending on the configuration.
    /// @param sequences The hailstone sequences whose colors are to be evaluated.
    /// @param frequencyMap A map containing the frequencies of each unique segment.
    /// @return A map containing each channel as a string with the ith index of the vector being the ith segment's channel value for that color.
    std::unordered_map<std::string, std::vector<uint8_t>> getStyles(const std::vector<std::vector<uint64_t>> &sequences, const std::unordered_map<std::string, uint32_t> &frequencyMap);

    /// @brief Exits the process and terminates it gracefully.
    void quit();
};
#endif