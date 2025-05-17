#include "collatz_subproc_header.hpp"

Utilities::Utilities() {};

RGBA ConfigUtilities::getRGBA(const std::string &rgbaHex) {
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

Gradient ConfigUtilities::getGradient(const std::string &gradientHexes) {
    std::vector<std::string> rgbaHexes = StringUtilities::split(gradientHexes, ",");
    if (rgbaHexes.size() != 2) {
        throw std::invalid_argument("Invalid gradient format.");
    }
    for (int i = 0; i < rgbaHexes.size(); ++i) {
        rgbaHexes[i] = StringUtilities::strip(rgbaHexes[i]);
    }
    Gradient gradient = {getRGBA(rgbaHexes[0]), getRGBA(rgbaHexes[1])};
    return gradient;
}

uint32_t ConfigUtilities::getValue(const std::string &strValue) {
    return static_cast<uint32_t>(std::stoul(strValue));
}

ImageDimensions ConfigUtilities::getDimensions(const std::string &imageSize) {
    std::vector<std::string> dims = StringUtilities::split(imageSize, "x");
    for (int i = 0; i < dims.size(); ++i) {
        dims[i] = StringUtilities::strip(dims[i]);
    }
    if (dims.size() != 2) {
        throw std::invalid_argument("Invalid image dimensions format.");
    }
    ImageDimensions dimensions = {getValue(dims[0]), getValue(dims[1])};
    return dimensions;
}

F32 ConfigUtilities::getFloatValue(const std::string &strFloat)  {
    return static_cast<F32>(std::stof(strFloat));
}

std::unordered_map<std::string, std::string> ConfigUtilities::getConfig(const fs::path &configPath) {
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

std::vector<std::string> StringUtilities::split(const std::string &str, const std::string &delimiter) {
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

std::string StringUtilities::strip(const std::string &str) {
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

fs::path ConfigUtilities::getExecutablePath() {
    std::vector<char> buffer(MAX_PATH);
    DWORD len = GetModuleFileNameA(NULL, buffer.data(), buffer.size());
    if (len == 0) {
        throw std::runtime_error("Path of executable cannot be retrieved.");
    }
    std::string strPath =  "";
    strPath.assign(buffer.data(), len);
    return fs::path(strPath);
}

Range SubprocessUtilities::getRange(const std::string &rangeStr) {
    std::vector<std::string> rangeStrVal = StringUtilities::split(rangeStr, " ");
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


 HSVA Utilities::getHSVA(const RGBA &rgba) {
    static const size_t channelCount = 4;
    std::vector<F32> normalizedRGB(channelCount - 1);
    for (size_t i = 0; i < channelCount - 1; ++i) {
        normalizedRGB[i] = rgba[i] / static_cast<F32>(255);
    }
    const F32 maxValue = getMax(normalizedRGB);
    const F32 minValue = getMin(normalizedRGB);
    const F32 chroma = maxValue - minValue;
    F32 hue = 0.0f;
    if (chroma > 0.0f) {
        if (maxValue == normalizedRGB[0]) {
            const F32 hueP = (normalizedRGB[1] - normalizedRGB[2]) / chroma;
            hue = 60.0f * (hueP + ((hueP < 0.0f) ? 6.0f : 0.0f));
        } else if (maxValue == normalizedRGB[1]) {
            hue = 60.0f * (((normalizedRGB[2] - normalizedRGB[0]) / chroma) + 2.0f);
        } else {
            hue = 60.0f * (((normalizedRGB[0] - normalizedRGB[1]) / chroma) + 4.0f);
        }   
        hue /= 360;
    }
    F32 saturation = maxValue > 0 ? chroma / maxValue : 0;
    F32 value = maxValue;
    HSVA hsva = {hue, saturation, value, rgba[3] / 255.0f};
    return hsva;
 }

 F32 Utilities::getHue(const std::unordered_map<std::string, std::vector<F32>> &coordinates, const std::vector<uint32_t> &frequencyMap, bool isFrequencyBased) {
    return 0.0;
 }
 F32 Utilities::getSaturation(const std::unordered_map<std::string, std::vector<F32>> &coordinates, const std::vector<uint32_t> &frequencyMap, bool isFrequencyBased) {
    return 0.0;
 }
 F32 Utilities::getValue(const std::unordered_map<std::string, std::vector<F32>> &coordinates, const std::vector<uint32_t> &frequencyMap, bool isFrequencyBased) {
    return 0.0;
 }

 RGBA Utilities::getRGBA(const HSVA &hsva) {
    if (hsva[1] == 0.0f) {
        const uint8_t gray = static_cast<uint8_t>(std::round(hsva[2] * 255.0f));
        RGBA values = {gray, gray, gray, static_cast<uint8_t>(round(hsva[3] * 255))};
        return values;
    } else {
        F32 hP = hsva[0] * 6.0f;
        if (hP >= 6.0) {
            hP = 0.0;
        } 
        const F32 fractional = hP - std::floor(hP);
        const F32 p = hsva[2] * (1 - hsva[1]);
        const F32 q = hsva[2] * (1 - hsva[1] * fractional);
        const F32 t = hsva[2] * (1.0 - hsva[1] * (1 - fractional));
        F32 r = 0.0f, g = 0.0f, b = 0.0f;
        switch (static_cast<uint32_t>(std::floor(hP))) {
            case 0: r = hsva[2]; g = t; b = p; break;
            case 1: r = q; g = hsva[2]; b = p; break;
            case 2: r = p; g = hsva[2]; b = t; break;
            case 3: r = p; g = q;b = hsva[2]; break;
            case 4: r = t; g = p; b = hsva[2]; break;
            case 5: r = hsva[2]; g = p; b = q; break;
        }
        RGBA values = {
            static_cast<uint8_t>(std::round(r * 255)),
            static_cast<uint8_t>(std::round(g * 255)),
            static_cast<uint8_t>(std::round(b * 255)),
            static_cast<uint8_t>(std::round(hsva[3] * 255))
        };
        return values;
    }
 }

std::unordered_map<std::string, uint32_t> Utilities::getFrequencyMap(const std::vector<std::vector<uint64_t>> &sequences) {
    std::unordered_map<std::string, uint32_t> frequencyMap = {};
    for (std::vector<uint64_t> seq : sequences) {
        const size_t seqSize = seq.size();
        for (size_t i = 1; i < seqSize; ++i) {
            const std::string repr = getStrRepr(seq[i - 1], seq[i]);
            if (frequencyMap.find(repr) != frequencyMap.end()) {
                frequencyMap[repr] += 1;
            } else {
                frequencyMap[repr] = 1;
            }
        }
    }
    return frequencyMap;
 }

std::string Utilities::getStrRepr(const uint64_t a, const uint64_t b) {
    std::string repr(sizeof(uint64_t) * 2, '\0');
    std::memcpy(repr.data(), &a, sizeof(uint64_t));
    std::memcpy(repr.data() + sizeof(uint64_t), &b, sizeof(uint64_t));
    return repr;
}

std::array<uint64_t, 2> Utilities::getIntsFromRepr(const std::string &repr) {
    std::array<uint64_t, 2> integers = {0, 0};
    std::memcpy(&integers[0], repr.data(), sizeof(uint64_t));
    std::memcpy(&integers[1], repr.data() + sizeof(uint64_t), sizeof(uint64_t));
    return integers;
}