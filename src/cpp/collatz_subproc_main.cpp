#include "collatz_subproc_header.hpp"

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    _setmode(_fileno(stderr), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    #endif
    std::unique_ptr<Utilities> utils = std::make_unique<Utilities>();
    std::unique_ptr<IO> io = std::make_unique<IO>();
    std::unique_ptr<Subprocess> subproc = std::make_unique<Subprocess>(std::move(utils), std::move(io));
    subproc->start();
    return 0;
}

Subprocess::Subprocess(std::unique_ptr<Utilities> utilities, std::unique_ptr<IO> io) : 
    utilities(std::move(utilities)),
    io(std::move(io)) {};

void Subprocess::start() {
    fs::path configPath = utilities->getExecutablePath().parent_path() / "config.yaml";
    config = utilities->getConfig(configPath);
    Range range = io->getRange();

    // TODO: Process Data.
    // Store result then signal 
    std::cerr << processFinished << '\n';
    std::cerr.flush();

    // Send data in binary
    std::string sendCode = "";
    std::getline(std::cin, sendCode);
    if (sendCode == sendData) {
        // 0xA is a newline (\n)
        std::vector<uint8_t> data = {0x00, 0x01, 0xA}; 
        std::cout.write(reinterpret_cast<const char*>(data.data()), data.size());
        std::cout.flush();
    } else {
        std::cout << sendCode;
        std::cout.flush();
    }
}