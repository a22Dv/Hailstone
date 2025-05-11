#include "collatz_header.hpp"


/// @brief Gets the final values to evaluate based on the range specified in the user's input, as well as a config file.
/// @param userInput The user input range.
/// @return A vector holding all final values to be evaluated.
std::vector<uint32_t> Utilities::getValuesToEvaluate(
    const std::string &userInput, 
    const std::unordered_map<std::string, std::string> &config
) {
    const std::vector<std::string> rangeStr = splitByChar(' ', userInput);
    const std::pair<uint32_t, uint32_t> range = {std::stoi(rangeStr[0]), std::stoi(rangeStr[1])};
    const std::string rangeSelect =  std::get<std::string>(extractFromConfig("rangeSelect", config));
    const uint32_t numSamples = std::get<uint32_t>(extractFromConfig("numSamples", config));
    const size_t numberOfValues =  range.second - range.first > numSamples ? numSamples : range.second - range.first;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t>  dist(range.first, range.second);
    std::vector<uint32_t> values(numberOfValues);
    if (rangeSelect == "random" && range.second - range.first > numSamples) {
        for (size_t i = 0; i < numberOfValues; ++i) {
            values[i] = dist(gen);
        }
    } else {
        for (size_t i = 0; i < numberOfValues; ++i) {
            values[i] = i;
        }
    }
    return values;
}

ConfigContainer Utilities::extractFromConfig(
    const std::string &field, 
    const std::unordered_map<std::string, std::string> &config
) {
    ConfigContainer returnValue;
    static const std::unordered_set<std::string> numberFilter = {"angleOdd", "angleEven", "numSamples"};
    if (numberFilter.find(field) != numberFilter.end()) {
        returnValue = getNumber(config.at(field));
    } else if (field == "gradientColors") {
        returnValue = getGradientColor(config.at(field));
    } else if (field == "backgroundColor") {
        returnValue = getColor(config.at(field));
    } else if (field == "imageSize") {
        returnValue = getImageSize(config.at(field));
    } else if (field == "configMetaDataOnImage") {
        returnValue = getBool(config.at(field));
    } else {
        returnValue = config.at(field);
    }
    return returnValue;
};

/// @brief Returns split strings by a specified `char` delimeter.
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


/// @brief Wraps a string in predetermined syntax for messages to be received by the calling process.
/// @param message The `std::string` to be wrapped.
/// @return A wrapped string.
std::string Utilities::wrapMessage(const std::string &message) {
    std::stringstream ss;
    ss << messageStart << message << endDelimiter << messageStart;
    return ss.str();
}

/// @brief Wraps the sequences in a predetermined syntax for sequences to be received by the calling process.
/// @param sequences The sequences to be converted to string and wrapped.
/// @return The wrapped and string-converted sequences.
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

uint32_t Utilities::getNumber(const std::string &str) {
    return std::stoi(str);
}

std::vector<uint8_t> Utilities::getColor(const std::string &str) {
    uint32_t colorBytes = std::stoi(str);
    std::vector<uint8_t> rgba = {
        static_cast<uint8_t>((colorBytes >> 24) & 0xFF),
        static_cast<uint8_t>((colorBytes >> 16) & 0xFF),
        static_cast<uint8_t>((colorBytes >> 8) & 0xFF),
        static_cast<uint8_t>(colorBytes & 0xFF),
    };
    return rgba;
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> Utilities::getGradientColor(const std::string &str) {
    std::vector<std::string> strColors = splitByChar(',', str);
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> colors = {getColor(strColors[0]), getColor(strColors[1])};
    return colors;
}

std::pair<uint32_t, uint32_t> Utilities::getImageSize(const std::string &str) {
    std::vector<std::string> strDimensions = splitByChar('x', str);
    std::pair<uint32_t, uint32_t> dimensions = {std::stoi(strDimensions[0]), std::stoi(strDimensions[1])};
    return dimensions;
}

bool Utilities::getBool(const std::string &str) {
    if (str == "true") {
        return true;
    } else {
        return false;
    }
}

std::unordered_map<std::string, std::string> Utilities::getConfig()
{
    return std::unordered_map<std::string, std::string>();
}
