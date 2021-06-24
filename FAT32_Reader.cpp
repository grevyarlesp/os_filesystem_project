//
// Created by user on 6/24/2021.
//

#include "FAT32_Reader.h"


bool FAT32_Reader:: readBootSector() {
    p_boot = (pFat_BS_T) malloc(512);
    DWORD bytesRead;
    SetFilePointer(device, 0, nullptr, FILE_BEGIN);
    return ReadFile(device, p_boot, 512, &bytesRead, nullptr) && p_boot->bootable_signature==0xAA55;
}

FAT32_Reader::FAT32_Reader(HANDLE device): Filesystem_Reader(device) {
    if (! readBootSector()) {
        std::wcout << "Failed to read FAT32 boot sector\n";
        return;
    }
    if (! readRoot()) {
        std::wcout << "Failed to read FAT32 root directory\n";
        return;
    }
}

bool FAT32_Reader::readSector(unsigned int sectorToRead, int sectorCount, uint8_t *sector) {
    DWORD bytesRead;
    int retCode = 0;
    SetFilePointer(device, sectorToRead * p_boot->bytes_per_sector, nullptr, FILE_BEGIN);//Set a Point to Read
    return ReadFile(device, sector, sectorCount * p_boot->bytes_per_sector, &bytesRead, nullptr);
}


bool FAT32_Reader::readRoot() {
    unsigned int root_dir_sector = 0;
    unsigned int first_data_sector =
            p_boot->reserved_sector_count + (p_boot->table_count * p_boot->table_size_32) + root_dir_sector;
    unsigned int root_cluster = p_boot->root_cluster_number;
    unsigned int size = sizeof(DirectoryEntry) * p_boot->root_entry_count;
    unsigned int first_sector_of_cluster = ((root_cluster - 2) * p_boot->sectors_per_cluster) + first_data_sector;
    unsigned int current_cluster = root_cluster;
    unsigned int current_sector = first_data_sector;
    unsigned int current_byte = current_sector * p_boot->bytes_per_sector;
    DWORD byteCount;

    BYTE *buffer = (BYTE *) malloc(512);
    auto pLongFileName1 = (pLongFileName) malloc(sizeof(LongFileName));
    bool lfn = false;
    bool finished = false;
    auto pDirEntry1 = (pDirEntry) malloc(sizeof(DirEntry));
    current_byte = first_sector_of_cluster * p_boot->bytes_per_sector;
    std::wstring tmp[20];
    while (!finished) {
        if (!readSector(current_sector, 1, buffer)) {
            return false;
        }
        int last = -1;
        for (int i = 0, pos = 0; i < p_boot->bytes_per_sector / sizeof(DirEntry); ++i, pos += sizeof(DirEntry)) {
            if (buffer[pos] == 0x00) {
                finished = true;
                break;
            }
            if (buffer[pos] == 0xE5) {
                // Unused
                continue;
            }
            if (buffer[pos + 11] == 0x0F) { // Long File Name
                pLongFileName1 = (pLongFileName) (buffer + pos);
                lfn = true;
                if (((pLongFileName1->order & 0xF0) >> 4) == 4) last = pLongFileName1->order & 0x0F;
                int order = (pLongFileName1->order & 0x0F) - 1;
                for (int i = 0; i < 5; ++i) {
                    if (pLongFileName1->name[i] == 0xFFFF) {
                        break;
                    }
                    tmp[order].push_back(pLongFileName1->name[i]);
                }
                for (int i = 0; i < 6; ++i) {
                    if (pLongFileName1->name2[i] == 0xFFFF) {
                        break;
                    }
                    tmp[order].push_back(pLongFileName1->name2[i]);
                }
                for (int i = 0; i < 2; ++i) {
                    if (pLongFileName1->name3[i] == 0xFFFF) {
                        break;
                    }
                    tmp[order].push_back(pLongFileName1->name3[i]);
                }
            } else {  // Not Long File Name
                if (lfn) { // Have a long file name in storage
                    for (int j = 0; j < last; ++j) {
                        std::wcout << tmp[j];
                        tmp[j].clear();
                    }
                    lfn = false;
                    std::wcout << '\n';
                    continue;
                }
                pDirEntry1 = (pDirEntry) (buffer + pos);
                printStr(pDirEntry1->name, 0, 8);
                std::wcout << ".";
                printStr(pDirEntry1->name, 8, 3);
                std::wcout << '\n';
            }
        }
        current_sector += 1;
    }
    free(buffer);
    return true;
}

FAT32_Reader::~FAT32_Reader() {
    free(p_boot);
    free(pDir);

}
