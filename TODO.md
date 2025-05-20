# TODO

- Setup batching on both sides for extremely large ranges. Might need additions to the IPC protocol. Instead of giving large ranges, divide the ranges into chunks to send to the process, avoiding modifications to the C++ code. Then build the string up from there within Python to process into batches once again for the GPU. (ModernGL)

- Rewrite or at the very least significantly refactor the script. C++ work is "almost" done? Maybe there are a few more lurking bugs though. But it compiles.