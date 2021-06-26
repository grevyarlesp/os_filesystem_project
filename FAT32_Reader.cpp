#include "FAT32_Reader.h"

bool FAT32_Reader:: readBootSector() {
    p_boot = (pFat_BS_T) malloc(512);
    DWORD bytesRead;
    SetFilePointer(device, 0, nullptr, FILE_BEGIN);
    return ReadFile(device, p_boot, 512, &bytesRead, nullptr) && p_boot->bootable_signature==0xAA55;
}

FAT32_Reader::FAT32_Reader(HANDLE device): Filesystem_Reader(device) {
    prev_sector = 0;
    if (! readBootSector()) {
        std::wcout << "Failed to read FAT32 boot sector\n";
        return;
    }

    first_data_sector =
            p_boot->reserved_sector_count + (p_boot->table_count * p_boot->table_size_32) + 0; // + root_dir_sector = 0
    if (! readRoot()) {
        std::wcout << "Failed to read FAT32 root directory\n";
        return;
    }
    if (! readFat()) {
        std::wcout << "Failed to read the file allocation table\n";
        return;
    }
}

bool FAT32_Reader::readFat() {
    return true;
}

bool FAT32_Reader::readSector(unsigned int sectorToRead, int sectorCount, uint8_t *sector) {
    DWORD bytesRead;
    int retCode = 0;
    SetFilePointer(device, (LONG) sectorToRead * p_boot->bytes_per_sector, nullptr, FILE_BEGIN);//Set a Point to Read
    return ReadFile(device, sector, (LONG) sectorCount * p_boot->bytes_per_sector, &bytesRead, nullptr);
}

bool FAT32_Reader::readRoot() {
    unsigned int root_dir_sector = 0;
    unsigned long current_sector = first_data_sector;
    unsigned int root_cluster = p_boot->root_cluster_number;
    unsigned int first_sector_of_cluster = ((root_cluster - 2) * p_boot->sectors_per_cluster) + first_data_sector;
    this->cur_sector = current_sector;
    return readDirectory(first_sector_of_cluster, this->v_items);
}

bool FAT32_Reader::enterDirectory(unsigned int cluster) {
    for (auto i : v_items) { delete i; }

    v_items.clear();
    unsigned long sector = first_data_sector + ((cluster - 2) * p_boot->sectors_per_cluster);
    prev_sector = cur_sector;
    cur_sector = sector;
    return readDirectory(sector, this->v_items);
}

bool FAT32_Reader::readDirectory(unsigned long start_sector, std::vector<Item*>& v) {
    unsigned int current_sector = start_sector;
    DWORD byteCount;
    BYTE *buffer = (BYTE *) malloc(512);
    auto pLongFileName1 = (pLongFileName) malloc(sizeof(LongFileName));
    bool lfn = false;
    bool finished = false;
    auto pDirEntry1 = (pDirEntry) malloc(sizeof(DirEntry));
    std::wstring tmp[20];
    std::wstring tmps;
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
                pDirEntry1 = (pDirEntry) (buffer + pos);
                uint32_t cluster = (pDirEntry1->first_cluster_high << 16) | (pDirEntry1->first_cluster_low);
                cluster = cluster & 0x0FFFFFFF; // Ignore the first 4 bits
                if (lfn) { // Have a long file name in storage
                    for (int j = 0; j < last; ++j) {
                        tmps.append(tmp[j]);
                        tmp[j].clear();
                    }
//                    std::wcout << tmps;
                    lfn = false;
//                    std::wcout << '\n';
                    if (pDirEntry1->attrib==0x10) { // Directory
                        v.push_back(new Directory(tmps, cluster));
                    } else {
                        v.push_back(new File(tmps, cluster, pDirEntry1->size));
                    }

                    tmps.clear();
                    continue;
                }
                printStr(tmps, pDirEntry1->name, 0, 8);
                tmps.push_back('.');
                printStr(tmps, pDirEntry1->name, 8, 3);
//                std::wcout << tmps;
//                std::wcout << '\n';
                if (pDirEntry1->attrib==0x10) { // Directory
                    v.push_back(new Directory(tmps, cluster));
                } else {
                    v.push_back(new File(tmps, cluster, pDirEntry1->size));
                }
                tmps.clear();
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

void FAT32_Reader::printCurrentDirectory() {
    for (size_t i = 0; i < v_items.size(); ++i) {
        std::wcout << i << ' ' <<  v_items[i]->getName() << '\n';
    }
}

void FAT32_Reader::openItem() {

}
