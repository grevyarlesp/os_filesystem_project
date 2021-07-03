#include <windows.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <io.h>
#include "FAT32_Reader.h"
#include "NTFS_Reader.h"

HANDLE device;

int main(int argc, char **argv) {
    _setmode(_fileno(stdout), 0x00020000); // _O_U16TEXT
    std::string drive = R"(\\.\F:)";
    if (argc != 3) return 0;
    drive[4] = argv[1][0];
    Filesystem_Reader* pReader;
    device = CreateFile(drive.c_str(),    // Drive to open
                        GENERIC_READ,           // Access mode
                        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
                        nullptr,                   // Security Descriptor
                        OPEN_EXISTING,          // How to create
                        0,                      // File attributes
                        nullptr);                  // Handle to template
    //FAT32 -> 0 else NTFS
    if (argv[2] == 0) {
        pReader = new FAT32_Reader(device);
        pReader->printBootInformation();
        pReader->printCurrentDirectory();
        unsigned int k;
        while (true) {
            std::wcout << "Enter the item number to open: ";
            std::wcin >> k;
            pReader->openItem(k);
            std::wcout << "DONE\n";
        }
    } else {
        pReader = new NTFS_Reader(device);
        pReader->printBootInformation();
        pReader->printCurrentDirectory();
        unsigned int k;
        while (true) {
            std::wcout << "Enter the item number to open: ";
            std::wcin >> k;
            pReader->openItem(k);
            std::wcout << "DONE\n";
        }

    }
    return 0;
}