#include "collatz_subproc_header.hpp"

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
    std::array<std::string, 10> settings = {
        "mode", "sampleSize", "scaling", "angleIfOdd", "angleIfEven", 
        "colorScheme", "backgroundColor", "gradient", "colorBasedOn", "imageSize"
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