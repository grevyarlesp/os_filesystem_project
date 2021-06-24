#ifndef FILESYSTEM_VIEWER_FAT32_READER_H
#define FILESYSTEM_VIEWER_FAT32_READER_H

#include "Filesystem_Reader.h"

class FAT32_Reader : public Filesystem_Reader {
public:
    explicit FAT32_Reader(HANDLE device);
private:
    pFat_BS_T p_boot;
    pDirEntry pDir;
    bool readSector(unsigned int sectorToRead, int sectorCount, uint8_t *sector);
    bool readBootSector();
    bool readRoot();
    ~FAT32_Reader();
};


#endif //FILESYSTEM_VIEWER_FAT32_READER_H
