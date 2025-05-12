#include "collatz_subproc_header.hpp"

int main(int argc, char* argv[]) {
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
    int x = 0;
}