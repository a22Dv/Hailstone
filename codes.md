# IPC Codes

- `/1` - Processing finished. (C++ sends to Python, Python gets entire line.)
- `/2` - Send image data.

*Sending data from one process to another via `subprocess.PIPE` should always end in a newline `\n`, when receiving a message, always consume the newline from the message. This makes the interface for both consistent, despite differences in behavior between C++'s `std::getline` and Python's `subprocess.stdout.readline()`*
# Data Format

- First 4 bytes correspond to number of segments.
- Data comes in the following format:

     - x1 - 4 bytes
     - y1 - 4 bytes
     - x2 - 4 bytes
     - y2 - 4 bytes
     - r - 1 byte
     - g - 1 byte
     - b - 1 byte
     - a - 1 byte
     - t *(Line thickness)* - 1 byte 
    - Total: 25 bytes/segment





