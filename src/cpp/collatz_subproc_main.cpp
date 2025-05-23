#include "collatz_subproc_header.hpp"

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    _setmode(_fileno(stderr), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    #endif
    std::unique_ptr<IPC> ipc = std::make_unique<IPC>(false);
    std::unique_ptr<Subprocess> subproc = std::make_unique<Subprocess>(std::move(ipc));
    subproc->start();
    return 0;
}

Subprocess::Subprocess(std::unique_ptr<IPC> ipc) : 
    ipc(std::move(ipc)) {};

void Subprocess::start() {
    fs::path configPath = ConfigUtilities::getExecutablePath().parent_path() / "config.yaml";
    config = ConfigUtilities::getConfig(configPath);
    std::stringstream ss;

    while (true) {
        const std::string input = ipc->receive();
        if (input == ipc->codes.at("terminate")) {
            quit();
        } else if (input == ipc->codes.at("test")) {
            ipc->send(ipc->codes.at("testSuc"), false);
            continue;
        }
        const Range range = SubprocessUtilities::getRange(input);

        ipc->send("Setting values...\n", false);
        const std::vector<uint32_t> values = getValues(range);

        ss << "Values set.\nNo. of sequences to evaluate: " << values.size() << "\n";
        ipc->send(ss.str(), false);
        ss.str("");
        ipc->send("Evaluating sequences...", false);
        const std::vector<std::vector<uint64_t>> sequences = getSequences(values);

        size_t seg_size = 0;
        for (std::vector<uint64_t> seq : sequences) {
            seg_size += seq.size() - 1; 
        }
        ss << "Sequences evaluated.\nNo. of coordinates to set: " << seg_size * 8 << " values.\n";
        ipc->send(ss.str(), false);
        ss.str("");
        ipc->send("Evaluating coordinates...", false);
        const std::unordered_map<std::string, std::vector<float>> coordinates = getCoordinates(sequences);
        const bool getMap = config.at("color-based-on") == "Frequency-based" && config.at("color-scheme") == "Gradient";
        
        if (getMap) { 
            ipc->send("Getting frequency map...", false); 
        }
        const std::unordered_map<std::string, uint32_t> frequencyMap = (
            getMap ? 
            SubprocessUtilities::getFrequencyMap(sequences) :
            std::unordered_map<std::string, uint32_t>()
        );

        ipc->send("Getting styles...", false);
        const std::unordered_map<std::string, std::vector<uint8_t>> styles = getStyles(sequences, frequencyMap);

        ipc->send("Assembling values...", false);
        const std::string imageData = SubprocessUtilities::assembleValues(
            coordinates, styles, 
            ConfigUtilities::getRGBA(config.at("background-color"))
        );

        ss << ipc->codes.at("procFnsh") << imageData.size();
        ipc->send(ss.str(), false);
        ss.str("");
        const std::string code = ipc->receive();
        if (code == ipc->codes.at("sendData")) {
            ipc->send(imageData, true);
        } else {
            ipc->send(ipc->codes.at("failureToReceive"), true);
        }
        ss.str("");
    }
}
std::vector<uint32_t> Subprocess::getValues(const Range &range) {
    if (range.first == range.second) {
        std::vector<uint32_t> singleValue = {range.first};
        return singleValue;
    }
    static const std::string mode = config.at("mode");
    const size_t effectiveRange = static_cast<size_t>(range.second - range.first);
    const uint32_t sampleSize = ConfigUtilities::getValue(config.at("sample-size"));
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
    static const uint8_t lineLength = static_cast<uint8_t>(ConfigUtilities::getValue(config.at("line-length")));
    static const uint8_t lineWidth = static_cast<uint8_t>(ConfigUtilities::getValue(config.at("line-width")));
    static const std::vector<std::string> parameters = {"x1", "x2", "x3", "x4", "y1", "y2", "y3", "y4"};
    static const size_t parameterCount = parameters.size();
    static const F32 decay = 0.99;
    static float angleIfOdd = MathUtilities::getRadians(ConfigUtilities::getFloatValue(config.at("angle-if-odd")));
    static float angleIfEven = MathUtilities::getRadians(ConfigUtilities::getFloatValue(config.at("angle-if-even")));
    static bool isLogarithmic = scaling == "Logarithmic";
    const size_t sequencesSize = sequences.size();
    std::unordered_map<std::string, std::vector<F32>> coordinates = {};
    std::vector<std::vector<F32>*> coordinatePtrs(parameterCount);
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
        F32 currentLineLength = lineLength;
        const std::vector<uint64_t>& sequence = sequences[i];
        const size_t sequenceSize = sequence.size();
        const F32 initialTheta = MathUtilities::getRadians(90.0);
        F32 currentTheta = initialTheta;
        (*coordinatePtrs[0])[sequenceStartIndex] = 0.0;
        (*coordinatePtrs[4])[sequenceStartIndex] = 0.0;

        for (size_t j = sequenceSize - 1; j > 0; --j) { // Moves backward, Starts at 1 in the sequence, until the sequence ends at N.
            const size_t vectorIndex = sequenceStartIndex + sequenceSize - 1 - j; // The last segment in the sequence, (1 -> 2) is the first index for the coordinates.
            const F32 theta = (sequence[j] & 0b1 == 0b1 ? angleIfOdd : angleIfEven) + currentTheta;
            (*coordinatePtrs[1])[vectorIndex] = (*coordinatePtrs[0])[vectorIndex] + currentLineLength * std::cosf(theta);
            (*coordinatePtrs[5])[vectorIndex] = (*coordinatePtrs[4])[vectorIndex] + currentLineLength * std::sinf(theta);
            (*coordinatePtrs[2])[vectorIndex] = (*coordinatePtrs[1])[vectorIndex] + lineWidth * std::cosf(MathUtilities::getRadians(90) + theta);
            (*coordinatePtrs[6])[vectorIndex] = (*coordinatePtrs[5])[vectorIndex] + lineWidth * std::sinf(MathUtilities::getRadians(90) + theta);
            (*coordinatePtrs[3])[vectorIndex] = (*coordinatePtrs[0])[vectorIndex] + lineWidth * std::cosf(MathUtilities::getRadians(90) + theta);
            (*coordinatePtrs[7])[vectorIndex] = (*coordinatePtrs[4])[vectorIndex] + lineWidth * std::sinf(MathUtilities::getRadians(90) + theta);
            if (j > 1) {
                (*coordinatePtrs[0])[vectorIndex + 1] = (*coordinatePtrs[1])[vectorIndex];
                (*coordinatePtrs[4])[vectorIndex + 1] = (*coordinatePtrs[5])[vectorIndex];
            }
            if (isLogarithmic) {
                currentLineLength *= decay;
            }   
            currentTheta = theta;
        }
        sequenceStartIndex += sequenceSize - 1;
    }
    return coordinates;
}

std::unordered_map<std::string, std::vector<uint8_t>> Subprocess::getStyles(
    const std::vector<std::vector<uint64_t>> &sequences, 
    const std::unordered_map<std::string, uint32_t> &frequencyMap
) {
    static const std::string colorScheme = config.at("color-scheme");
    static const bool isFlat = colorScheme == "Flat";
    static const RGBA backgroundColor = ConfigUtilities::getRGBA(config.at("background-color"));
    static const Gradient gradient = ConfigUtilities::getGradient(config.at("gradient"));
    static const std::string colorBasedOn = config.at("color-based-on");
    static const bool isFrequencyBased = colorBasedOn == "Frequency-based";
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
            for (size_t j = 0; j < segmentCount; ++j) {
                (*colorPtrs[i])[j] = gradient.first[i];
            }
        }
    } else {
        const HSVA startingColor = ColorUtilities::RGBAToHSVA(gradient.first);
        const HSVA endingColor = ColorUtilities::RGBAToHSVA(gradient.second);
        if (isFrequencyBased) {
            const std::unordered_map<std::string, uint32_t> rankings = SubprocessUtilities::getSegmentsRankings(frequencyMap, sequences);
            size_t startingIndex = 0;
            const uint32_t rankingPositions = rankings.size();
            for (std::vector<uint64_t> seq : sequences) {
                const size_t seqSize = seq.size();
                for (size_t i = 0; i < seqSize - 1; ++i) {
                    const size_t vectorIndex = startingIndex + i;
                    const size_t seqIndex = seqSize - i - 2;
                    const std::string repr = SubprocessUtilities::getStrRepr(seq[seqIndex], seq[seqIndex + 1]);
                    RGBA color = ColorUtilities::getRGBASegmentValue(startingColor, endingColor, rankings.at(repr), rankingPositions);
                    (*colorPtrs[0])[vectorIndex] = color[0];
                    (*colorPtrs[1])[vectorIndex] = color[1];
                    (*colorPtrs[2])[vectorIndex] = color[2];
                    (*colorPtrs[3])[vectorIndex] = color[3];
                }
                startingIndex += seqSize - 1;
            }
        } else { // Length-based
            size_t startingIndex = 0;
            for (std::vector seq : sequences) {
                const size_t seqSize = seq.size();
                for (size_t i = 0; i < seqSize; ++i) {
                    const size_t vectorIndex = startingIndex  + i;
                    RGBA color = ColorUtilities::getRGBASegmentValue(startingColor, endingColor, i, seqSize - 1);
                    (*colorPtrs[0])[vectorIndex] = color[0];
                    (*colorPtrs[1])[vectorIndex] = color[1];
                    (*colorPtrs[2])[vectorIndex] = color[2];
                    (*colorPtrs[3])[vectorIndex] = color[3];
                }
                startingIndex += seqSize;
            }
        }
    }
    return colors;
}

void Subprocess::quit() {
    exit(0);
}

