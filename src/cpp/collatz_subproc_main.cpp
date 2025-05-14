#include "collatz_subproc_header.hpp"

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    _setmode(_fileno(stderr), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    #endif
    std::unique_ptr<Utilities> utils = std::make_unique<Utilities>();
    std::unique_ptr<IPC> ipc = std::make_unique<IPC>();
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

void Subprocess::quit() {
    exit(0);
}