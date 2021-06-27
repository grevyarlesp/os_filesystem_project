#ifndef FILESYSTEM_VIEWER_FILESYSTEM_READER_H
#define FILESYSTEM_VIEWER_FILESYSTEM_READER_H
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <string>

void printStr(std::wstring& ws, const char *s, int l, int num);

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
    uint8_t _reserved; // Flags for Windows NT, or reserved
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
    wchar_t name[5];
    uint8_t attrib;// 0x0F
    uint8_t type; //
    uint8_t checksum;
    wchar_t name2[6];
    uint16_t zero; // always zero
    wchar_t name3[2];
} *pLongFileName, LongFileName;
#pragma pack()

class Filesystem_Reader {
protected:
    HANDLE device;
public:
    explicit Filesystem_Reader(HANDLE device);
    virtual void printCurrentDirectory() = 0;
    virtual void openItem(int item_number) = 0;
    virtual void printBootInformation() = 0;
};

#endif //FILESYSTEM_VIEWER_FILESYSTEM_READER_H
