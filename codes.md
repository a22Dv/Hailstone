# IPC Codes

- `/1` - Processing finished. (C++ sends to Python, Python gets entire line.)
- `/2` - Send image data.

*Sending data from one process to another via `subprocess.PIPE` should always end in a newline `\n`, when receiving a message, always consume the newline from the message. This makes the interface for both consistent, despite differences in behavior between C++'s `std::getline` and Python's `subprocess.stdout.readline()`*
# Data Format

- Background Color (RGBA) (Byte 0->3)
- Repeating for each line segment:
     - x1 - 4 Bytes (F32)
     - x2 - 4 Bytes (F32)
     - x3 - 4 Bytes (F32)
     - x4 - 4 Bytes (F32)
     - y1 - 4 Bytes (F32)
     - y2 - 4 Bytes (F32)
     - y3 - 4 Bytes (F32)
     - y4 - 4 Bytes (F32)
     - r - 1 Byte (uint8_t)
     - g - 1 Byte (uint8_t)
     - b - 1 Byte (uint8_t)
     - a - 1 Byte (uint8_t)
- Total per segment: 36 bytes
 



