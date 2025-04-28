#ifndef COMPUTE_COLLATZ_HPP_
#define COMPUTE_COLLATZ_HPP_
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdint>
class Collatz {
private:
    std::vector<int64_t> get_sequence(int32_t seed);
    std::vector<std::string> splitString(std::string str, char delimiter=' ');
public:
    void main_process();
};
#endif