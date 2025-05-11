#include "collatz_header.hpp"

std::atomic<bool> running(true);

/// @brief Application entry point.
/// @param argc Argument Count.
/// @param argv Argument Vector.
int main(int argc, char* argv[]) {

    // Can be stack-allocated, which is easier. This is just a practice for using pointers.
    std::unique_ptr<Utilities> utils = std::make_unique<Utilities>();
    std::unique_ptr<Application> app = std::make_unique<Application>(std::move(utils));
    app->start();
    return 0;
}

/// @brief Handles external SIGTERM to exit process.
/// @param sigNum To receive SIGTERM (15)
void handleSigterm(int sigNum) {
    running = false;
}

/// @brief Constructor for `Application`.
/// @param utils A `Utilities` instance to use.
Application::Application(std::unique_ptr<Utilities> utils) : utilInstance(std::move(utils)) {};

/// @brief Main process orchestrator.
void Application::start() {

    // Process successfully started.
    std::cout << utilInstance->wrapMessage("SUCCESS");
    std::signal(SIGTERM, handleSigterm);
    config = utilInstance->getConfig();

    while (running) {

        // Get input.
        std::string userInput = "";
        std::getline(std::cin, userInput);

        // Process request.
        std::vector<uint32_t> range = utilInstance->getValuesToEvaluate(userInput, config);
        std::vector<std::vector<uint64_t>> sequences = {};
        for (uint32_t n : range) {
            sequences.push_back(getSequence(n));
        }
    }
}

/// @brief Returns a vector holding the collatz sequence for a number `n`.
/// @param n The value to get the sequence of.
/// @return An `std::vector<uint64_t>` holding the collatz sequence for `n`.
std::vector<uint64_t> Application::getSequence(uint32_t n) {
    std::vector<uint64_t> sequence = {n};
    uint64_t currentNumber = static_cast<uint64_t>(n);
    while (currentNumber != 1) {
        if (currentNumber & 1) {
            currentNumber = 3 * currentNumber + 1;
        } else {
            currentNumber /= 2;
        }
        sequence.push_back(currentNumber);
    }
    return sequence;
}

