#include <windows.h>
#include <stdio.h>
#include <iostream>

#pragma pack(1)
typedef struct fat_BS {
    unsigned char bootjmp[3];
    unsigned char oem_name[8];
    unsigned short bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t table_count;
    uint16_t root_entry_count;
    unsigned short total_sectors_16;
    unsigned char media_type;
    uint16_t table_size_16;
    unsigned short sectors_per_track;
    unsigned short head_side_count;
    unsigned int hidden_sector_count;
    unsigned int total_sectors_32;
    //extended fat32 stuff
    unsigned int table_size_32; // fat32 table size
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

typedef struct DirectoryEntry {
    char name[11];
    uint8_t attrib;
    uint8_t reserved;
    uint8_t creation_time_dcs;
    uint16_t creation_date;
    uint16_t creation_time;
    uint16_t last_accessed;
    uint16_t first_cluster_high;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t first_cluster_low;
    uint32_t size;
} *pDirEntry, DirEntry;

typedef struct LongFileName {
    uint8_t order;
    WCHAR name[5];
    uint8_t attrib;// 0x0F
    uint8_t type; //
    uint8_t checksum;
    WCHAR name2[6];
    uint16_t zero; // always zero
    WCHAR name3[2];
} *pLongFileName, LongFileName;
#pragma pack()

HANDLE device;

pFat_BS_T pFat_boot;
pDirEntry pDir;
uint8_t *pFat;


void printStr(const char* s, int l, int num) {
    for (int i = l; i < l + num; ++i) {
        std::cout << s[i];
    }
}

void printWStr(const WCHAR* s, int l, int num) {
    for (int i = l; i < l + num; ++i) {
        std::wcout << s[i];
    }
}

bool readBootSector(HANDLE device) {
    pFat_boot = (pFat_BS_T) malloc(512);
    DWORD bytesRead;
    SetFilePointer(device, 0, NULL, FILE_BEGIN);
    return ReadFile(device, pFat_boot, 512, &bytesRead, NULL);
}

bool readSector(HANDLE device, int sectorToRead, int sectorCount, uint8_t *sector) {
    DWORD bytesRead;
    int retCode = 0;

    SetFilePointer(device, sectorToRead * pFat_boot->bytes_per_sector, NULL, FILE_BEGIN);//Set a Point to Read
    if (!ReadFile(device, sector, sectorCount * pFat_boot->bytes_per_sector, &bytesRead, NULL)) {
        //printf("ReadFile: %u\n", GetLastError());
        return 0;
    } else {
        //printf("Success!\n");
        return 1;
    }
}


bool readRDET(HANDLE device) {
    unsigned int root_dir_sector = 0;
    unsigned int first_data_sector =
            pFat_boot->reserved_sector_count + (pFat_boot->table_count * pFat_boot->table_size_32) + root_dir_sector;
    unsigned int root_cluster = pFat_boot->root_cluster_number;
    unsigned int size = sizeof(DirectoryEntry) * pFat_boot->root_entry_count;
    unsigned int first_sector_of_cluster = ((root_cluster - 2) * pFat_boot->sectors_per_cluster) + first_data_sector;
    unsigned int current_cluster = root_cluster;
    unsigned int current_sector = first_data_sector;
    unsigned int current_byte = current_sector * pFat_boot->bytes_per_sector;
    DWORD byteCount;

    BYTE buffer[512];
    auto pLongFileName1 = (pLongFileName) malloc(sizeof(LongFileName));
    bool lfn = false;
    bool finished = false;
    auto pDirEntry1 = (pDirEntry) malloc(sizeof(DirEntry));
//    std::cout << first_sector_of_cluster << '\n';
     current_byte = first_sector_of_cluster * pFat_boot->bytes_per_sector;
    // readSector(device, 29536, 1, buffer);
    std::wstring tmp;
    std::cout << sizeof(LongFileName) << '\n';

    while (! finished) {
        SetFilePointer(device, current_sector * pFat_boot->bytes_per_sector, NULL, FILE_BEGIN);
        readSector(device, current_sector, 1, buffer);
        for (int i = 0, pos = 0; i < pFat_boot->bytes_per_sector / sizeof(DirEntry); ++i, pos += sizeof(DirEntry)) {
            if (buffer[pos] == 0) {
                finished = true;
                break;
            }
             if (buffer[pos] == 0xE5) {
                 // Unused
                 continue;
             }
             if (buffer[pos + 11] == 0x0F) { // Long File Name
                 pLongFileName1 = (pLongFileName) (buffer + pos);
                 //std::cout << "Long file name: ";
                 for (int i = 0; i < 5; ++i) {
                    tmp.push_back(pLongFileName1->name[i]);
                 }
                 for (int i = 0; i < 6; ++i) {
                     tmp.push_back(pLongFileName1->name2[i]);
                 }
                 for (int i = 0; i < 2; ++i) {
                     tmp.push_back(pLongFileName1->name3[i]);
                 }
                 lfn = true;
             } else {  // Not Long File Name
                 pDirEntry1 = (pDirEntry) (buffer + pos);
                 //std::cout << pDirEntry1->name << '\n';
                 printStr(pDirEntry1->name, 0, 8);
                 std::cout << ".";
                 if (lfn) {
                     std::wcout << tmp << '\n';
                     lfn = false;
                     tmp.clear();
                 }
                 printStr(pDirEntry1->name, 8, 3);
                 std::cout << '\n';
             }
        }
        current_sector += 1;
    }

    return true;
}

bool readFat(HANDLE device) {
    pFat = (uint8_t *) malloc(pFat_boot->table_size_32 * pFat_boot->bytes_per_sector);
    //return readSector(device, pFat_boot->__reserved, pFat_boot->bytes_per_sector, (BYTE*) pFat);
}

int main(int argc, char **argv) {
    device = CreateFile("\\\\.\\F:",    // Drive to open
                        GENERIC_READ,           // Access mode
                        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
                        NULL,                   // Security Descriptor
                        OPEN_EXISTING,          // How to create
                        0,                      // File attributes
                        NULL);                  // Handle to template

    if (!readBootSector(device)) {
        std::cout << "Failed to read disk\n";
        return -1;
    } else if (pFat_boot->bootable_signature == 0xAA55)
        std::cout << "Bootable signature found ! Success!!!\n";
    std::cout << (int) pFat_boot->table_size_32 << '\n';

    if (!readRDET(device)) {
        std::cout << "Failed to read RDET";
        return -1;
    }

    return 0;
}