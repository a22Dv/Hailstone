#include "collatz_subproc_header.hpp"

Range IO::getRange() {
    std::string rangeStr = "";
    std::getline(std::cin, rangeStr);
    std::vector<std::string> rangeStrVal = Utilities::split(rangeStr, " ");
    if (rangeStrVal.size() != 2) {
        throw std::invalid_argument("Invalid range format received.");
    }
    Range range = {std::stoul(rangeStrVal[0]), std::stoul(rangeStrVal[1])};
    return range;
}