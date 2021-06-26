#ifndef FILESYSTEM_VIEWER_FAT32_READER_H
#define FILESYSTEM_VIEWER_FAT32_READER_H

#include "Filesystem_Reader.h"
#include "Item.h"
#include <vector>
#include <unordered_map>

class FAT32_Reader : public Filesystem_Reader {
public:
    explicit FAT32_Reader(HANDLE device);
    void printCurrentDirectory() override;
    void openItem(int item_number);
    void printBootInformation();
private:
    std::vector<Item*> v_items;
    pFat_BS_T p_boot;
    pDirEntry pDir;
    unsigned int* fat_value;
    unsigned int prev_sector;
    unsigned int cur_sector;
    unsigned long first_data_sector;
    bool readSector(unsigned int sectorToRead, int sectorCount, uint8_t *sector);
    bool readBootSector();
    bool readRoot();
    bool readFat();
    bool readDirectory(unsigned long start_sector, std::vector<Item*> &v);
    bool enterDirectory(unsigned int cluster);

    ~FAT32_Reader();

};


#endif //FILESYSTEM_VIEWER_FAT32_READER_H
