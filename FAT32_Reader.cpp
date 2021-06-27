#include "FAT32_Reader.h"

bool FAT32_Reader:: readBootSector() {
    p_boot = (pFat_BS_T) malloc(512);
    DWORD bytesRead;
    SetFilePointer(device, 0, nullptr, FILE_BEGIN);
    return ReadFile(device, p_boot, 512, &bytesRead, nullptr) && p_boot->bootable_signature==0xAA55;
}

FAT32_Reader::FAT32_Reader(HANDLE device): Filesystem_Reader(device) {
    prev_cluster = 0;
    if (! readBootSector()) {
        std::wcout << "Failed to read FAT32 boot sector\n";
        return;
    }
    first_data_sector =
            p_boot->reserved_sector_count + (p_boot->table_count * p_boot->table_size_32) + 0; // + root_dir_sector = 0
    first_fat_sector = p_boot->reserved_sector_count;
    if (! readRoot()) {
        std::wcout << "Failed to read FAT32 root directory\n";
        return;
    }
}

unsigned int FAT32_Reader::readFat(unsigned int active_cluster) {
    auto sector_size = p_boot->bytes_per_sector;
    unsigned char FAT_table[sector_size];
    unsigned int fat_offset = active_cluster * 4;
    unsigned int fat_sector = first_fat_sector + (fat_offset  / sector_size);
    unsigned int ent_offset = fat_offset / sector_size;
    readSector(fat_sector, 1, FAT_table);
    unsigned int table_value = *(unsigned int*)&FAT_table[ent_offset] & 0x0FFFFFFF;
    return table_value;
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

    this->cur_cluster = root_cluster;

    return readDirectory(root_cluster, this->v_items);
}

bool FAT32_Reader::enterDirectory(unsigned int cluster) {
    for (auto i : v_items) { delete i; }
    v_items.clear();
    prev_cluster = cur_cluster;
    cur_cluster = cluster;
    return readDirectory(cluster, this->v_items);
}

bool FAT32_Reader::readDirectory(uint32_t active_cluster, std::vector<Item*>& v) {
    unsigned int current_sector;
    DWORD byteCount;
    BYTE *buffer = (BYTE *) malloc(512);
    auto pLongFileName1 = (pLongFileName) malloc(sizeof(LongFileName));
    bool lfn = false;
    bool finished = false;
    auto pDirEntry1 = (pDirEntry) malloc(sizeof(DirEntry));
    std::wstring tmp[256];
    std::wstring tmps;
    uint32_t cnt = p_boot->sectors_per_cluster;
    while (! finished && active_cluster < 0x0FFFFFF7) { // bad cluster or end of chain
        current_sector = (active_cluster - 2) * p_boot->sectors_per_cluster + first_data_sector;
        uint8_t last = 0;
        while (cnt-- && !finished) {
            if (!readSector(current_sector, 1, buffer)) {
                return false;
            }
            for (int i = 0, pos = 0; i < p_boot->bytes_per_sector / sizeof(DirEntry); ++i, pos += sizeof(DirEntry)) {
                if (buffer[pos] == 0x00) {
                    finished = true;
                    break;
                }
                if (buffer[pos] == 0xE5) {
                    // Unused
                    continue;
                }
                pDirEntry1 = (pDirEntry) (buffer + pos);
                uint32_t cluster = (pDirEntry1->first_cluster_high << 16) | (pDirEntry1->first_cluster_low);
                cluster = cluster & 0x0FFFFFFF; // Ignore the first 4 bits
                if (buffer[pos + 11] == 0x0F) { // Long File Name
                    pLongFileName1 = (pLongFileName) (buffer + pos);
                    lfn = true;
                    // First byte: 0 -> first, 4 -> last
                    if (pLongFileName1->order & 0x40) { // last long entry
                        last = pLongFileName1->order ^ 0x40;
                    }
                    uint8_t order = (pLongFileName1->order & 63) - 1;
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
                            tmps.append(tmp[j]);
                            tmp[j].clear();
                        }
                        lfn = false;
                        if (pDirEntry1->attrib == 0x10) { // Directory
                            v.push_back(new Directory(tmps, cluster, pDirEntry1->attrib));
                        } else {
                            v.push_back(new File(tmps, cluster, pDirEntry1->size, pDirEntry1->attrib));
                        }
                        last = 0;
                        tmps.clear();
                        continue;
                    }
                    printStr(tmps, pDirEntry1->name, 0, 8);
                    tmps.push_back('.');
                    printStr(tmps, pDirEntry1->name, 8, 3);
                    if (pDirEntry1->attrib == 0x10) { // Directory
                        v.push_back(new Directory(tmps, cluster, pDirEntry1->attrib));
                    } else {
                        v.push_back(new File(tmps, cluster, pDirEntry1->size, pDirEntry1->attrib));
                    }
                    tmps.clear();
                }
            }
            current_sector += 1;
        }
        active_cluster = readFat(active_cluster);
    }

    free(buffer);
    return true;
}


FAT32_Reader::~FAT32_Reader() {
    free(p_boot);
    free(pDir);
    delete[] fat_table;
}

void FAT32_Reader::printCurrentDirectory() {
    for (size_t i = 0; i < v_items.size(); ++i) {
        std::wcout << i << L' ' <<  v_items[i]->getName() << L'|' << v_items[i]->getSize() << L'|' << v_items[i]->getAttribute() << '\n'; ;
    }
}

void FAT32_Reader::readCurrentFile(uint32_t active_cluster) {

}

void FAT32_Reader::openItem(int item_number) {
    if (v_items[item_number]->isDirectory()) {
        enterDirectory(v_items[item_number]->getFirstCluster());
    } else {

    }
}

void FAT32_Reader::printBootInformation() {
    std::wcout << "OEM NAME: ";
    for (int i = 0; i < 8; ++i)
        std::wcout << (char) p_boot->oem_name[i] ;
    std::wcout << '\n';
    std::wcout << "Bytes per sector: " << p_boot->bytes_per_sector << '\n';
    std::wcout << "Sector per cluster: " << (int)p_boot->sectors_per_cluster << '\n';
    std::wcout << "Number of sectors reserved for boot: " << p_boot->reserved_sector_count << '\n';
    std::wcout << "Number of tables: " << p_boot->table_count << '\n';
    std::wcout << "Media type: " << p_boot->media_type << '\n';
    std::wcout << "Sectors per track: " << p_boot->sectors_per_track << '\n';
    std::wcout << "Head side count: " << p_boot->head_side_count << '\n';
    std::wcout << "Hidden sectors count: " << p_boot->hidden_sector_count << '\n';
    std::wcout << "Total sectors: " << p_boot->total_sectors_32 << '\n';
    std::wcout << "Table size FAT32: " << p_boot->table_size_32 << '\n';
    std::wcout << "Extended flags: " << p_boot->flags << '\n';
    std::wcout << "Fat version: " << p_boot->fat_version << '\n';
    std::wcout << "First root cluster: " << p_boot->root_cluster_number << '\n';
    std::wcout << "Backup BS sector: " << p_boot->backup_boot_sector << '\n';
}


