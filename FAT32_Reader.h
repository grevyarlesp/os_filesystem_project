#ifndef FILESYSTEM_VIEWER_FAT32_READER_H
#define FILESYSTEM_VIEWER_FAT32_READER_H

#include "Filesystem_Reader.h"
#include "Item.h"
#include <vector>
#include <stack>
#include <unordered_map>

class FAT32_Reader : public Filesystem_Reader {
public:
    explicit FAT32_Reader(HANDLE device);
    void printCurrentDirectory() override;
    void openItem(int item_number) override;
    void printBootInformation() override;
private:
    std::vector<Item*> v_items;

    uint32_t *fat_table;
    unsigned int* fat_value;

    pFat_BS_T p_boot;
    pDirEntry pDir;
    std::stack<uint32_t> s_dir;
    unsigned int prev_cluster;
    unsigned int cur_cluster;
    unsigned long first_data_sector;
    unsigned int first_fat_sector;
    bool readSector(unsigned int sectorToRead, int sectorCount, uint8_t *sector);
    bool readBootSector();
    bool readRoot();

    unsigned int readFat(unsigned int active_cluster);
    bool readDirectory(uint32_t  active_cluster, std::vector<Item*> &v);
    bool enterDirectory(unsigned int cluster);
    ~FAT32_Reader();


    void readCurrentFile(uint32_t first_cluster);
};


#endif //FILESYSTEM_VIEWER_FAT32_READER_H
