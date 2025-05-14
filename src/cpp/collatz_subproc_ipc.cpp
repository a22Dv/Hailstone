#include "collatz_subproc_header.hpp"

IPC::IPC(bool text) : text(text) {};

void IPC::send(const std::string &message, bool stdOut = true) {
    // std::string can act as byte containers hence only std::string is used.
    if (stdOut) {
        std::cout << message << codes.at("send");
        std::cout.flush();
    } else {
        std::cerr << message << codes.at("send");
        std::cerr.flush();
    }
}

std::string IPC::receive() {
    std::string stream = "";
    std::getline(std::cin, stream);
    return stream;
}
