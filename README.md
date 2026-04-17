# Cross-Platform File Sender

A high-performance TCP file transfer application written in C++ with a Python-based TUI wrapper. Supports cross-platform communication (Windows <-> Linux) and handles large files (>4GB) correctly.

## 📜 Credits & Attribution

- **Original Core Logic:** Created by **BratGronflo**.
- **Contributions & Fixes:** This repository contains significant bug fixes, optimizations, and feature additions implemented by me.

### What was fixed/improved:
1.  **Critical Bug Fixes:**
    -   Fixed `sizeof(pointer)` error in metadata transmission that caused corruption.
    -   Resolved `uint32_t` overflow issue, enabling support for files larger than 4GB.
    -   Fixed partial send/receive logic to prevent data loss on slow networks.
2.  **Cross-Platform Compatibility:**
    -   Refactored code to work seamlessly on both Windows (Winsock) and Linux/Unix (POSIX Sockets).
    -   Added conditional compilation headers (`Network.h`).
3.  **Performance Optimizations:**
    -   Increased buffer size from 4KB to 1MB for higher throughput.
    -   Implemented `TCP_NODELAY` to reduce latency.
    -   Added reliable `sendAll`/`recvAll` helpers.
4.  **User Interface:**
    -   Added a modern Python TUI (Text User Interface) using `rich` for real-time progress tracking and logs.

---

## 🚀 Features

-   **Large File Support:** Correctly handles files > 4GB using 64-bit integers.
-   **Cross-Platform:** Send files from Windows to Linux and vice versa.
-   **Real-time Progress:** Shows transfer speed (MB/s) and progress bar.
-   **Robust Protocol:** Handles metadata (filename, extension, size) reliably.

## 🛠️ Compilation

### Prerequisites
-   C++11 compatible compiler (GCC, Clang, or MSVC).

### Build from Source (Linux/macOS)

```bash
g++ -std=c++11 -o file_transfer main.cpp Client.cpp Server.cpp FileCopier.cpp
```

### Build from Source (Windows)

Using MinGW:
```bash
g++ -std=c++11 -o file_transfer.exe main.cpp Client.cpp Server.cpp FileCopier.cpp -lws2_32
```

Or simply open the project in Visual Studio.

### Project structure

- main.cpp: Entry point.
- Network.h: Cross-platform socket abstraction and helper functions.
- Client.cpp/h: Client-side logic.
- Server.cpp/h: Server-side logic.
- FileCopier.cpp/h: File I/O handling.

### Notes

- Ensure port 4444 is open in your firewall if transferring over a network.
- When testing locally, do not save the received file with the same name as the source file in the same directory to avoid data corruption. 
- Python TUI is is development. tui_app.py not working on this moment
