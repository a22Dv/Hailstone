#include "compute_collatz_header.hpp"

void Collatz::main_process() {
    while (true) {
        std::string stdin_input;
        std::getline(std::cin, stdin_input);
        std::vector<std::string> proc_input = splitString(stdin_input, '|');
        std::pair<int64_t, int64_t> args = {std::stoi(proc_input[0]), std::stoi(proc_input[1])};

    }
}
std::vector<int64_t> Collatz::get_sequence(int32_t seed) {
    
}
std::vector<std::string> Collatz::splitString(std::string str, char delimiter) {

}