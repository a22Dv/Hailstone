# TODO

- Setup batching on both sides for extremely large ranges. Might need additions to the IPC protocol. Instead of giving large ranges, divide the ranges into chunks to send to the process, avoiding modifications to the C++ code. Then build the string up from there within Python to process into batches once again for the GPU. (ModernGL)
- Calculate the coordinates for the segments needed. Work started at:
`std::unordered_map<std::string, std::vector<float>> Subprocess::getCoordinates(const std::vector<std::vector<uint64_t>> &sequences)`
- Calculate the styling required for each of the segments in RGBA, still empty.
