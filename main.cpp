#include <windows.h>
#include <stdio.h>
#include <iostream>

#pragma pack(1)
typedef struct fat_BS {
    unsigned char bootjmp[3];
    unsigned char oem_name[8];
    unsigned short bytes_per_sector;
    uint8_t sectors_per_cluster;
    unsigned short reserved_sector_count;
    unsigned char table_count;
    unsigned short root_entry_count;
    unsigned short total_sectors_16;
    unsigned char media_type;
    unsigned short table_size_16;
    unsigned short sectors_per_track;
    unsigned short head_side_count;
    unsigned int hidden_sector_count;
    unsigned int total_sectors_32;
    //extended fat32 stuff
    unsigned int sectors_per_fat;
    unsigned short flags;
    unsigned short fat_version;
    unsigned int root_cluster_number;
    unsigned short fsinfo_sector_number;
    unsigned short backup_boot_sector;
    uint8_t reserved[12];
    uint8_t disk_number;
    uint8_t __reserved; // Flags for Windows NT, or reserved
    uint8_t signature;
    unsigned int volume_id_serial;
    char volume_label[11];
    char system_identifier[8];
    uint8_t boot_code[420];
    unsigned short bootable_signature;
} *pFat_BS_T;
#pragma pack()

HANDLE device;

pFat_BS_T pFat_boot;
uint8_t* pFat;

bool readBootSector(HANDLE device) {
    pFat_boot = (pFat_BS_T) malloc(512);
    DWORD bytesRead;
    SetFilePointer(device, 0, NULL, FILE_BEGIN);
    return ReadFile(device, pFat_boot, 512, &bytesRead, NULL);
}

bool readSector(HANDLE device, int readPoint, int sectorCount, uint8_t* sector) {
    DWORD bytesRead;
    int retCode = 0;
    if (device == INVALID_HANDLE_VALUE) {
        printf("Error: %u\n", GetLastError());
        return 0;
    }
    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read
    if (!ReadFile(device, sector, sectorCount * pFat_boot->bytes_per_sector, &bytesRead, NULL)) {
        //printf("ReadFile: %u\n", GetLastError());
        return 0;
    } else {
        //printf("Success!\n");
        return 1;
    }
}



bool readFat(HANDLE device) {
    pFat = (uint8_t*) malloc(pFat_boot->sectors_per_fat * pFat_boot->bytes_per_sector);
    return readSector(device, pFat_boot->__reserved, pFat_boot->bytes_per_sector, (BYTE*) pFat);
}

int main(int argc, char **argv) {
    device = CreateFile("\\\\.\\F:",    // Drive to open
                        GENERIC_READ,           // Access mode
                        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
                        NULL,                   // Security Descriptor
                        OPEN_EXISTING,          // How to create
                        0,                      // File attributes
                        NULL);                  // Handle to template

    if (! readBootSector(device)) {
        std::cout << "Failed to read disk\n";
        return -1;
    } else
        if (pFat_boot->bootable_signature == 0xAA55)
            std::cout << "Bootable signature found ! Success!!!\n";
    std::cout << pFat_boot->oem_name << '\n';
    std::cout << (int) pFat_boot->sectors_per_fat << '\n';
    if (! readFat(device)) {
        std::cout << "Reading FAT failed!!";
        return -1;
    }

    return 0;
}