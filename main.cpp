#include <windows.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <io.h>
#include "FAT32_Reader.h"

HANDLE device;

void hexdump(BYTE *buffer, int num) {
    for (int i = 0; i < num; ++i) {
        std::wcout << std::hex << (int) buffer[i];
        std::wcout << ' ';
    }
}

int main(int argc, char **argv) {
    _setmode(_fileno(stdout), 0x00020000); // _O_U16TEXT
    device = CreateFile(R"(\\.\F:)",    // Drive to open
                        GENERIC_READ,           // Access mode
                        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
                        nullptr,                   // Security Descriptor
                        OPEN_EXISTING,          // How to create
                        0,                      // File attributes
                        nullptr);                  // Handle to template

    Filesystem_Reader* fat32_reader = new FAT32_Reader(device);
    fat32_reader->printBootInformation();
    fat32_reader->printCurrentDirectory();
    fat32_reader->openItem(4);
//    fat32_reader->printCurrentDirectory();

//    fat32_reader->openItem(6);
    return 0;
}