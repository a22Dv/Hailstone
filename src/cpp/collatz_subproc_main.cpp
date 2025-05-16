#include "collatz_subproc_header.hpp"

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    _setmode(_fileno(stderr), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    #endif
    std::unique_ptr<Utilities> utils = std::make_unique<Utilities>();
    std::unique_ptr<IPC> ipc = std::make_unique<IPC>(false);
    std::unique_ptr<Subprocess> subproc = std::make_unique<Subprocess>(std::move(utils), std::move(ipc));
    subproc->start();
    return 0;
}

Subprocess::Subprocess(std::unique_ptr<Utilities> utilities, std::unique_ptr<IPC> ipc) : 
    utilities(std::move(utilities)),
    ipc(std::move(ipc)) {};

void Subprocess::start() {
    fs::path configPath = utilities->getExecutablePath().parent_path() / "config.yaml";
    config = utilities->getConfig(configPath);

    while (true) {
        const std::string input = ipc->receive();
        if (input == ipc->codes.at("terminate")) {
            quit();
        }
        const Range range = utilities->getRange(input);
        const std::vector<uint32_t> values = getValues(range);
        const std::vector<std::vector<uint64_t>> sequences = getSequences(values);
        const std::unordered_map<std::string, std::vector<float>> coordinates = getCoordinates(sequences);
        const std::unordered_map<std::string, std::vector<uint8_t>> styles = getStyles(sequences);
        const std::string imageData = utilities->assembleValues(coordinates, styles);
    
        ipc->send(ipc->codes.at("processingFinished"), false);
        const std::string code = ipc->receive();
        if (code == ipc->codes.at("sendData")) {
            ipc->send(imageData, true);
        } else {
            ipc->send(ipc->codes.at("failureToReceive"), true);
        }
    }
}
std::vector<uint32_t> Subprocess::getValues(const Range &range) {
    if (range.first == range.second) {
        std::vector<uint32_t> singleValue = {range.first};
        return singleValue;
    }
    static const std::string mode = config.at("mode");
    const size_t effectiveRange = static_cast<size_t>(range.second - range.first);
    const uint32_t sampleSize = utilities->getValue(config.at("sample-size"));
    if (mode == "Continuous" || effectiveRange < sampleSize) {
        std::vector<uint32_t> values(effectiveRange);
        for (size_t i = 0; i < effectiveRange; ++i) {
            values[i] = range.first + i;
        }
        return values;
    } else {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dist(range.first, range.second);
        std::vector<uint32_t> values(sampleSize);
        for (size_t i = 0; i < sampleSize; ++i) {
            values[i] = dist(gen);
        }
        return values;
    }
}
std::vector<std::vector<uint64_t>> Subprocess::getSequences(const std::vector<uint32_t> &values) {
    const size_t valueCount = values.size();
    std::vector<std::vector<uint64_t>> sequences(valueCount);
    for (size_t i = 0; i < valueCount; ++i) {
        sequences[i] = getSequence(values[i]);
    }
    return sequences;
}

std::vector<uint64_t> Subprocess::getSequence(uint32_t n) {
    uint64_t currentN = n;
    std::vector<uint64_t> sequence = {currentN};
    while (currentN != 1) {
        if (currentN & 0b1 == 0b1) {
            currentN = currentN * 3 + 1;
        } else {
            currentN /= 2;
        }
        sequence.push_back(currentN);
    }
    return sequence;
}

std::unordered_map<std::string, std::vector<F32>> Subprocess::getCoordinates(const std::vector<std::vector<uint64_t>> &sequences) {
    static const std::string scaling = config.at("scaling");
    static const uint8_t lineLength = static_cast<uint8_t>(utilities->getValue(config.at("line-length")));
    static const uint8_t lineWidth = static_cast<uint8_t>(utilities->getValue(config.at("line-width")));
    static const ImageDimensions imageDimensions = utilities->getDimensions(config.at("image-size"));
    static const std::vector<std::string> parameters = {"x1", "x2", "x3", "x4", "y1", "y2", "y3", "y4"};
    static const size_t parameterCount = parameters.size();
    static const F32 growth = 1.02;
    static float angleIfOdd = utilities->getRadians(utilities->getFloatValue(config.at("angle-if-odd")));
    static float angleIfEven = utilities->getRadians(utilities->getFloatValue(config.at("angle-if-even")));
    static bool isLogarithmic = scaling == "logarithmic";
    const size_t sequencesSize = sequences.size();
    std::unordered_map<std::string, std::vector<F32>> coordinates = {};
    std::vector<std::vector<F32>*> coordinatePtrs(parameterCount);
    F32 currentLineLength = lineLength;
    uint32_t segmentSum = 0;

    for (const std::vector<uint64_t> &sequence : sequences) {
        segmentSum += sequence.size() - 1;
    }
    for (size_t i = 0; i < parameterCount; ++i) {
        coordinates[parameters[i]] = std::vector<float>(segmentSum);
        coordinatePtrs[i] = &coordinates.at(parameters[i]);
    }

    size_t sequenceStartIndex = 0;
    for (size_t i = 0; i < sequencesSize; ++i) {
        const std::vector<uint64_t>& sequence = sequences[i];
        const size_t sequenceSize = sequence.size();

        (*coordinatePtrs[0])[sequenceStartIndex] = 0.0;
        (*coordinatePtrs[4])[sequenceStartIndex] = 0.0;

        for (size_t j = sequenceSize - 1; j > 0; --j) {
            const size_t vectorIndex = sequenceStartIndex + sequenceSize - 1 - j;
            const F32 theta = sequence[j] & 0b1 == 0b1 ? angleIfOdd : angleIfEven;
            (*coordinatePtrs[1])[vectorIndex] = (*coordinatePtrs[0])[vectorIndex] + currentLineLength * std::cosf(theta);
            (*coordinatePtrs[5])[vectorIndex] = (*coordinatePtrs[4])[vectorIndex] + currentLineLength * std::sinf(theta);
            (*coordinatePtrs[2])[vectorIndex] = (*coordinatePtrs[1])[vectorIndex] + lineWidth * std::cosf(utilities->getRadians(90) + theta);
            (*coordinatePtrs[6])[vectorIndex] = (*coordinatePtrs[5])[vectorIndex] + lineWidth * std::sinf(utilities->getRadians(90) + theta);
            (*coordinatePtrs[3])[vectorIndex] = (*coordinatePtrs[0])[vectorIndex] + lineWidth * std::cosf(utilities->getRadians(90) + theta);
            (*coordinatePtrs[7])[vectorIndex] = (*coordinatePtrs[4])[vectorIndex] + lineWidth * std::sinf(utilities->getRadians(90) + theta);
            if (j > 1) {
                (*coordinatePtrs[0])[vectorIndex + 1] = (*coordinatePtrs[1])[vectorIndex];
                (*coordinatePtrs[4])[vectorIndex + 1] = (*coordinatePtrs[5])[vectorIndex];
            }
            if (isLogarithmic) {
                currentLineLength *= growth;
            }   
        }
        sequenceStartIndex += sequenceSize - 1;
    }
    return coordinates;
}

std::unordered_map<std::string, std::vector<uint8_t>> Subprocess::getStyles(const std::vector<std::vector<uint64_t>> &sequences) {
    static const std::string colorScheme = config.at("color-scheme");
    static const bool isFlat = colorScheme == "Flat";
    static const RGBA backgroundColor = utilities->getRGBA(config.at("background-color"));
    static const Gradient gradient = utilities->getGradient(config.at("gradient"));
    static const std::string colorBasedOn = config.at("color-based-on");
    static const bool isFrequencyBased = colorBasedOn == "Frequency-Based";
    static const std::vector<std::string> components = {"r", "g", "b", "a"};
    static const size_t componentCount = components.size();
    std::unordered_map<std::string, std::vector<uint8_t>> colors = {};
    std::vector<std::vector<uint8_t>*> colorPtrs = {};
    size_t segmentCount = 0;
    for (const std::vector<uint64_t> &sequence : sequences) {
        segmentCount += sequence.size() - 1;
    }
    for (std::string comp : components) {
        colors[comp] = std::vector<uint8_t>(segmentCount);
        colorPtrs.push_back(&colors[comp]);
    }
    if (isFlat) {
        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < segmentCount; ++j)
            (*colorPtrs[i])[j] = gradient.first[i];
        }
    } else {
        /// @todo Gradient color scheme mapping and alculation, frequency/length based coloring.
        const HSVA startingColor = utilities->getHSVA(gradient.first);
        const HSVA endColor = utilities->getHSVA(gradient.second);
        
        /// @todo Define Gradient
        /// @todo Map gradient intermediaries to Length / Frequency depending on config.
    }
    return colors;
}

void Subprocess::quit() {
    exit(0);
}

