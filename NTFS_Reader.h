#ifndef FILESYSTEM_VIEWER_NTFS_READER_H
#define FILESYSTEM_VIEWER_NTFS_READER_H
// https://flatcap.org/linux-ntfs/ntfs/

#include "Filesystem_Reader.h"
#include "Item.h"
#include <vector>
#include <map>
#include <stack>

#define MFT_FILES_PER_BUFFER (65536)
#define MFT_FILE_SIZE (1024)

#pragma pack(1)
struct Standard_Information_Header {
    uint64_t ctime;
    uint64_t atime;
    uint64_t mtime;
    uint64_t rtime;
    uint32_t dos_perm;
    uint32_t max_ver;
    uint32_t ver;
    uint32_t class_id;
    uint32_t owner_id;
    uint32_t security_id;
    uint64_t quota;
    uint64_t usn;
};


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
    int8_t clusters_per_record;
    uint8_t unused[3];
    int8_t clusters_per_index;
    uint8_t unused3[3];
    uint64_t  serial;
    uint32_t checksum;
    // End of BPB
    uint8_t code[426];
    uint16_t end_marker;
} *pNTFS_BS_t;

typedef struct MFT_Record {
    char type[4]; // "FILE"
    uint16_t update_seq_offset;
    uint16_t update_seq_length;
    uint64_t log_file_seq_number;
    uint16_t record_seq_number;
    uint16_t hard_link_count;
    uint16_t attributes_offset;
    uint16_t flag;
    uint32_t bytes_in_use;
    uint32_t bytes_allocated;
    uint64_t base_record_number;
    uint16_t next_attribute_index;
    uint8_t reserved[2];
    uint32_t record_number;
} *pRecord, Record;



struct Attr_Header {
    uint32_t type;
    uint32_t length;
    uint8_t form_code;
    uint8_t attr_length;
    uint16_t attr_offset;
    uint16_t flag;
    uint16_t attr_id;
};


// 4 4 1 1 2 2 2 4 2 2
struct Attr_Header_R : Attr_Header { // resident;
    uint32_t content_length;
    uint16_t content_offset;
    uint8_t  indexed;
    uint8_t unused;
};

// 4 4 1 1 2 2 2 8 8 2 2 4 8 8 8
struct Attr_Header_NR : Attr_Header {
    uint64_t start_vcn;
    uint64_t end_vcn;
    uint16_t run_list_offset;
    uint16_t compress_size;
    uint32_t zero; // padding
    uint64_t content_disk_size;
    uint64_t content_size;
    uint64_t initial_content_size;
};

struct FileNameAttributeHeader : Attr_Header_R {
    uint64_t    parentRecordNumber : 48;
    uint64_t    sequenceNumber : 16;
    uint64_t    creationTime;
    uint64_t    modificationTime;
    uint64_t    metadataModificationTime;
    uint64_t    readTime;
    uint64_t    allocatedSize;
    uint64_t    realSize;
    uint32_t    flags;
    uint32_t    reparse;
    uint8_t     fileNameLength;
    uint8_t     namespaceType;
    wchar_t     fileName[1];
};
struct RunHeader {
    uint8_t     lengthFieldBytes : 4;
    uint8_t     offsetFieldBytes : 4;
};

#pragma pack()

struct FileS {
    uint64_t parent;
    std::wstring name;
    uint64_t start_sector;
    uint32_t record_number;
    uint64_t size;
    uint32_t flag;

};

struct Run {
    uint64_t offset;
    uint64_t length;

    Run() : offset(0LL), length(0LL) {};
    Run(uint64_t start_cluster, uint64_t length) : offset(start_cluster), length(length) {};
};


class NTFS_Reader : public Filesystem_Reader {

public:
    explicit NTFS_Reader(HANDLE device);
    void printCurrentDirectory() override;
    void openItem(int item_number) override;
    void printBootInformation() override;
    bool readBootSector();
private:
    std::stack<uint64_t> dir;

    std::map<uint32_t, std::vector<FileS>> v_items;
    std::map<uint32_t, FileS> m_files;

    pNTFS_BS_t p_boot;
    uint32_t sector_size;
    uint32_t cluster_size;
    uint32_t record_size;
    uint64_t total_cluster;
    uint8_t mftBuffer[MFT_FILES_PER_BUFFER * MFT_FILE_SIZE];
    void readRoot();
    uint8_t * readMFTRecord(uint64_t start);
    uint8_t *findAttribute(pRecord record, uint32_t type);
    bool readSector(uint32_t sectorToRead, int sectorCount, uint8_t *sector);
    uint64_t recordNumberToSector(uint32_t number);

    uint8_t *MFTDump();


    void fixRecord(uint8_t *buffer);
};

#endif //FILESYSTEM_VIEWER_NTFS_READER_H
