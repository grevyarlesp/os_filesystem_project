#ifndef FILESYSTEM_VIEWER_NTFS_READER_H
#define FILESYSTEM_VIEWER_NTFS_READER_H

#include "Filesystem_Reader.h"

#pragma pack(1)
typedef struct NTFS_BS {
    unsigned char bootjmp[3];
    unsigned char oem_name[8];
    // BPB
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint8_t reserved[2];
    uint8_t zero1[3]; // always 0
    uint16_t unused1; // not used
    unsigned char media_type;
    uint16_t zero2; // always 0
    uint16_t sectors_per_track;
    uint16_t head_side_count;
    unsigned int hidden_sector_count;
    uint64_t unused2; // Not used by NTFS
    uint64_t total_sectors_64;
    uint64_t MFT_cluster;
    uint64_t MFT_mirror_cluster;
    uint8_t clusters_per_record;
    uint8_t unused[3];
    uint8_t clusters_per_index;
    uint8_t unused3[3];
    uint64_t  serial;
    uint32_t checksum;
    // End of BPB
    uint8_t code[426];
    uint16_t end_marker;
} *pNTFS_BS_t;
typedef struct MFT_Record {
    char* type[4];
    uint16_t update_seq_offset;
    uint16_t update_seq_length;
    uint64_t log_file_seq_number;
    uint16_t record_seq_number;
    uint16_t hard_link_count;
    uint16_t attributes_offset;
    uint32_t bytes_in_use;
    uint32_t bytes_allocated;
    uint64_t parent_record_number;
    uint32_t next_atttibute_index;
    uint32_t reserved;
    uint64_t record_number;
} *pRecord, Record;
#pragma pack()

class NTFS_Reader : public Filesystem_Reader {
private:
    pNTFS_BS_t p_boot;
    int32_t sectors_per_record;
    int32_t sectors_per_index_buffer;

public:
    NTFS_Reader(HANDLE device) : Filesystem_Reader(device) {
        if (! readBootSector()) {
            std::wcout << "Failed to read NTFS boot sector\n";
            return;
        }
    }

    void printCurrentDirectory() override;
    void openItem(int item_number) override;
    void printBootInformation() override;
    bool readSector(unsigned int sectorToRead, int sectorCount, uint8_t *sector);
    bool readBootSector();
};


#endif //FILESYSTEM_VIEWER_NTFS_READER_H
