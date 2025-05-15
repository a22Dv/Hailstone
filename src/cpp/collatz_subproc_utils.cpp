#include "collatz_subproc_header.hpp"

Utilities::Utilities() {};

RGBA Utilities::getRGBA(const std::string &rgbaHex) {
    if (!(rgbaHex.length() == 9 && rgbaHex[0] == '#')) {
        throw std::invalid_argument("Invalid hex-code format for RGBA.");
    }
    const std::string code = rgbaHex.substr(1);
    uint32_t bytes = static_cast<uint32_t>(std::stoul(code, nullptr, 16));
    RGBA color = {
        static_cast<uint8_t>((bytes >> 24) & 0xFF),
        static_cast<uint8_t>((bytes >> 16) & 0xFF),
        static_cast<uint8_t>((bytes >> 8) & 0xFF),
        static_cast<uint8_t>((bytes) & 0xFF),
    };
    return color;
}

Gradient Utilities::getGradient(const std::string &gradientHexes) {
    std::vector<std::string> rgbaHexes = split(gradientHexes, ",");
    if (rgbaHexes.size() != 2) {
        throw std::invalid_argument("Invalid gradient format.");
    }
    for (int i = 0; i < rgbaHexes.size(); ++i) {
        rgbaHexes[i] = strip(rgbaHexes[i]);
    }
    Gradient gradient = {getRGBA(rgbaHexes[0]), getRGBA(rgbaHexes[1])};
    return gradient;
}

uint32_t Utilities::getValue(const std::string &strValue) {
    return static_cast<uint32_t>(std::stoul(strValue));
}

ImageDimensions Utilities::getDimensions(const std::string &imageSize) {
    std::vector<std::string> dims = split(imageSize, "x");
    for (int i = 0; i < dims.size(); ++i) {
        dims[i] = strip(dims[i]);
    }
    if (dims.size() != 2) {
        throw std::invalid_argument("Invalid image dimensions format.");
    }
    ImageDimensions dimensions = {getValue(dims[0]), getValue(dims[1])};
    return dimensions;
}

F32 Utilities::getFloatValue(const std::string &strFloat)  {
    return static_cast<F32>(std::stof(strFloat));
}

std::unordered_map<std::string, std::string> Utilities::getConfig(const fs::path &configPath) {
    std::ifstream yamlConfigFile(configPath);
    if (!yamlConfigFile.is_open()) {
        throw std::runtime_error("File cannot be opened.");
    }
    YAML::Node configFile = YAML::Load(yamlConfigFile);
    std::array<std::string, 12> settings = {
        "mode", "sample-size", "scaling", "angle-if-odd", "angle-if-even", 
        "color-scheme", "background-color", "gradient", "color-based-on", "image-size",
        "line-width", "line-length"
    };
    std::unordered_map<std::string, std::string> config = {};
    for (std::string setting : settings) {
        config[setting] = configFile[setting].as<std::string>();
    }
    return config;
}

std::vector<std::string> Utilities::split(const std::string &str, const std::string &delimiter) {
    std::vector<std::string> substrings = {};
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        substrings.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    substrings.push_back(str.substr(start));
    return substrings;
}

std::string Utilities::strip(const std::string &str) {
    const size_t strLen = str.length();
    bool frontHasSpace = true, backHasSpace = true;
    std::array<size_t, 2> range = {0, 0};
    for (size_t i = 0; i < strLen; ++i) {
        const size_t front = i, back = strLen - i - 1;
        if (!std::isspace(str[front]) && frontHasSpace) {
            frontHasSpace = false;
            range[0] = front;
        } 
        if (!std::isspace(str[back]) && backHasSpace) {
            backHasSpace = false;
            range[1] = back;
        }
        if (!frontHasSpace && !backHasSpace) {
            break;
        }
    }
    return str.substr(range[0], range[1] - range[0] + (frontHasSpace || backHasSpace ? 0 : 1));
}

fs::path Utilities::getExecutablePath() {
    std::vector<char> buffer(MAX_PATH);
    DWORD len = GetModuleFileNameA(NULL, buffer.data(), buffer.size());
    if (len == 0) {
        throw std::runtime_error("Path of executable cannot be retrieved.");
    }
    std::string strPath =  "";
    strPath.assign(buffer.data(), len);
    return fs::path(strPath);
}

Range Utilities::getRange(const std::string &rangeStr) {
    std::vector<std::string> rangeStrVal = Utilities::split(rangeStr, " ");
    if (rangeStrVal.size() != 2) {
        throw std::invalid_argument("Invalid range format received.");
    }
    Range range = {std::stoul(rangeStrVal[0]), std::stoul(rangeStrVal[1])};\
    if (range.first >= 1 && range.second >= 1) {
        return range;
    } else {
        throw std::invalid_argument("Invalid range format received.");
    }
    
}

std::string Utilities::assembleValues(
    const std::unordered_map<std::string, std::vector<F32>> &coordinates,
    const std::unordered_map<std::string, std::vector<uint8_t>> &style
 ) {
    static const std::vector<std::string> parameters = {"x1", "x2", "x3", "x4", "y1", "y2", "y3", "y4", "r", "g", "b", "a"};
    static const size_t parameterCount = parameters.size();
    static const size_t segmentByteCount = (sizeof(F32) * 8) + (sizeof(uint8_t) * 4);
    const size_t segmentCount = style.at("a").size();
    std::string assembledBuffer(segmentByteCount * segmentCount, '\0');
    char* bufferPtr = assembledBuffer.data();
    size_t byteIndex = 0;
    std::vector<const std::vector<F32>*> coordinateVector(8);
    std::vector<const std::vector<uint8_t>*> styleVector(5);
    for (size_t i = 0; i < 8; ++i) {
        coordinateVector[i] = &coordinates.at(parameters[i]);
    }
    for (size_t i = 8; i < 12; ++i) {
        styleVector[i - 8] = &style.at(parameters[i]);
    }
    for (size_t i = 0; i < segmentCount; ++i) {
        for (size_t j = 0; j < parameterCount; ++j) {
            bool isCoordinate = j < 8;
            const size_t sizeOfParameter = isCoordinate ? sizeof(F32) : sizeof(uint8_t);
            const char* byte = nullptr;
            if (isCoordinate) {
                byte = reinterpret_cast<const char*>(&(*coordinateVector[j])[i]);
            } else {
                byte = reinterpret_cast<const char*>(&(*styleVector[j - 8])[i]);
            }
            std::memcpy(bufferPtr + byteIndex, byte, sizeOfParameter);
            byteIndex += sizeOfParameter;
        }
    }
    return assembledBuffer;
 }

 F32 Utilities::getRadians(F32 degrees) {
    return degrees * (std::numbers::pi / 180);
 }

 HSV Utilities::getHSV(const RGBA &rgba) {
    static const size_t channelCount = 4;
    std::array<float, channelCount> normalizedRGBA = {};
    for (size_t i = 0; i < channelCount; ++i) {
        normalizedRGBA[i] = rgba[i] / static_cast<F32>(255);
    }

    // Get HSV, V is max(RGB), S = max-min / max || 0 if max == 0
    // H:( max = r -> 60 * (G - B)/delta mod 6, g -> 60 * (B - R/delta) + 2, b -> 60 *  R - G / delta + 4 )/ 360
 }
