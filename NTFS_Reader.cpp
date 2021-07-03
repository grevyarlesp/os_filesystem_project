#include "NTFS_Reader.h"
#include <tchar.h>
#include <fstream>

void hexdump(BYTE *buffer, int num) {
    for (int i = 0; i < num; ++i) {
        std::wcout << std::hex << buffer[i];
        std::wcout << ' ';
    }
}

uint64_t NTFS_Reader::recordNumberToSector(uint32_t number) {
    return number * record_size / p_boot->bytes_per_sector + p_boot->MFT_cluster * p_boot->sectors_per_cluster;
}

bool NTFS_Reader::readBootSector() {
    p_boot = (pNTFS_BS_t) malloc(512);
    DWORD bytesRead;

    SetFilePointer(device, 0, nullptr, FILE_BEGIN);
    if (! ReadFile(device, p_boot, 512, &bytesRead, nullptr) && p_boot->end_marker==0xAA55) {
        return false;
    }
//    std::wcout << p_boot->clusters_per_index;
//    std::wcout << '\n';
//    std::wcout << p_boot->clusters_per_record;
    return true;
}
bool NTFS_Reader::readSector(uint32_t sectorToRead, int sectorCount, uint8_t *sector) {
    DWORD bytesRead;
    int retCode = 0;
    LARGE_INTEGER pos;
    pos.QuadPart = sectorToRead * p_boot->bytes_per_sector;
    SetFilePointerEx(device, pos, nullptr, FILE_BEGIN);//Set a Point to Read
    return ReadFile(device, sector, (LONG) sectorCount * p_boot->bytes_per_sector, &bytesRead, nullptr);
}

// Find position of a attribute in a record
uint8_t* NTFS_Reader::findAttribute(pRecord record, uint32_t type) {

    uint8_t* p = ((uint8_t *) record) + record->attributes_offset;
    while (true) {
        if (p + sizeof(Attr_Header_R) > ((uint8_t*) record) + record_size)
            break;
        Attr_Header_R *header = (Attr_Header_R* ) p;
        if (header->type == 0xffffffff)
            break;
        if (header->type == type &&
        p + header->length <= ((uint8_t *) record) + record_size) {
            return p;
        }
        p += header->length;
    }

    return NULL;
}

void NTFS_Reader::printCurrentDirectory() {
    const auto &v = v_items[dir.top()];
    int i = 0;
    for (FileS f: v) {
        std::wcout << i << ' ' << f.start_sector << ' ' << f.name << L" | " << f.size << L'|';
        ++i;
        if (f.flag & 0x0001) {
            std::wcout << "Read-Only ";
        }
        if (f.flag & 0x0002)
            std::wcout << "Hidden ";
        if (f.flag & 0x0004)
            std::wcout << "System ";

        if (f.flag & 0x0020)
            std::wcout << "Archive ";

        if (f.flag & 0x0040)
            std::wcout << "Device ";

        if (f.flag & 0x0080)
            std::wcout << "Normal ";

        if (f.flag & 0x0100)
            std::wcout << "Temporary ";

        if (f.flag & 0x0200)
            std::wcout << "Sparse File ";
        if (f.flag & 0x0400)
            std::wcout << "Reparse Point ";
        if (f.flag & 0x0800)
            std::wcout << "Compressed ";

        if (f.flag & 0x1000)
            std::wcout << "Offline ";
        if (f.flag & 0x2000)
            std::wcout << "Not Content Indexed ";
        if (f.flag & 0x4000)
            std::wcout << "Encrypted ";
        if (f.flag & 0x10000000)
            std::wcout << "Directory ";
        std::wcout << '\n';
    }
}

void NTFS_Reader::openItem(int item_number) {
    uint64_t cur_dir = dir.top();
    const auto& v = v_items[cur_dir];
    if (item_number < 0 || item_number >= v.size())
        return;
    if (v[item_number].flag & 0x10000000) {
        std::wcout << "Opening directory..." << '\n';
        dir.push(v[item_number].record_number);
        std::wcout << v[item_number].record_number << '\n';
        this->printCurrentDirectory();
    } else {
        uint8_t* buffer = readMFTRecord(v[item_number].start_sector);
        pRecord fileRecord = (pRecord) buffer;
        Attr_Header *attribute = (Attr_Header*) (buffer + fileRecord->attributes_offset);
        Attr_Header *dataAttribute;
        std::string filename = "temp";

        std::ofstream wf;
        wf.open(filename, std::ios::out | std::ios::binary);

        while (true) {
            if (attribute->type == 0x80) {
                dataAttribute = (Attr_Header *) attribute;
            } else if (attribute->type == 0xFFFFFFFF) {
                break;
            }
            attribute = (Attr_Header *) ((uint8_t *) attribute + attribute->length);
        }
        if (dataAttribute->form_code == 0x01) {
            Attr_Header_NR *data = (Attr_Header_NR*) dataAttribute;
            uint64_t toRead = data->content_disk_size;
            RunHeader *dataRun = (RunHeader *) ((uint8_t *) data + data->run_list_offset);

            uint64_t clusterNumber = 0;
            while (((uint8_t *) dataRun - (uint8_t *) data) < data->length && dataRun->lengthFieldBytes) {
                uint64_t length = 0, offset = 0;
                for (int i = 0; i < dataRun->lengthFieldBytes; i++) {
                    length |= (uint64_t) (((uint8_t *) dataRun)[1 + i]) << (i * 8);
                }

                for (int i = 0; i < dataRun->offsetFieldBytes; i++) {
                    offset |= (uint64_t) (((uint8_t *) dataRun)[1 + dataRun->lengthFieldBytes + i])
                            << (i * 8);
                }

                if (offset & ((uint64_t) 1 << (dataRun->offsetFieldBytes * 8 - 1))) {
                    // Sign extend the offset.

                    for (int i = dataRun->offsetFieldBytes; i < 8; i++) {
                        offset |= (uint64_t) 0xFF << (i * 8);
                    }
                }
                clusterNumber += offset;
                dataRun = (RunHeader *) ((uint8_t *) dataRun + 1 + dataRun->lengthFieldBytes +
                                         dataRun->offsetFieldBytes);

                uint64_t remaining = length;
                uint8_t* bufer = new uint8_t[cluster_size];
                uint64_t pos = 0;
                while (toRead && remaining--) {
                    readSector(clusterNumber * p_boot->sectors_per_cluster + pos, p_boot->sectors_per_cluster, buffer);
                    wf.write((char*) buffer, std::min(toRead, (uint64_t) cluster_size));
                    toRead -= std::min(toRead, (uint64_t) cluster_size);
                    pos += p_boot->sectors_per_cluster;
                }
                delete[] buffer;
            }

        } else {
            Attr_Header_R *data = (Attr_Header_R*) dataAttribute;
            uint8_t* tmp = (uint8_t*) dataAttribute + data->content_offset;
            wf.write((char*) tmp, data->content_length);
            for (int i = 0; i < data->content_length; ++i, tmp += 1) {
                std::wcout << char(*tmp);
            }
        }
        wf.close();
        ShellExecute(nullptr, nullptr, filename.c_str(), nullptr, nullptr , SW_SHOW );
        std::wcout << '\n';
        delete[] buffer;
    }
}

void NTFS_Reader::printBootInformation() {
    std::wcout << "OEM NAME: ";
    for (int i = 0; i < 8; ++i)
        std::wcout << (char) p_boot->oem_name[i] ;
    std::wcout << '\n';
    std::wcout << "Bytes per sector: " << p_boot->bytes_per_sector << '\n';
    std::wcout << "Sector per cluster: " << (int)p_boot->sectors_per_cluster << '\n';
    std::wcout << "Sectors per track: " << p_boot->sectors_per_track << '\n';
    std::wcout << "Head side count: " << p_boot->head_side_count << '\n';
    std::wcout << "Hidden sectors count: " << p_boot->hidden_sector_count << '\n';
    std::wcout << "Total sectors: " << p_boot->total_sectors_64 << '\n';
    std::wcout << "MFT Cluster: " << p_boot->MFT_cluster << '\n';
    std::wcout << "Clusters/record: " << p_boot->clusters_per_record << '\n';
    std::wcout << "Bytes/record: " << record_size << '\n';

    readMFTRecord(p_boot->MFT_cluster * p_boot->sectors_per_cluster);
}

NTFS_Reader::NTFS_Reader(HANDLE device) : Filesystem_Reader(device) {
    if (! readBootSector()) {
        std::wcout << "Failed to read NTFS boot sector\n";
        return;
    }
    // https://www.cse.scu.edu/~tschwarz/coen252_07Fall/Lectures/NTFS.html
    sector_size = p_boot->bytes_per_sector;
    cluster_size = sector_size * p_boot->sectors_per_cluster;
    record_size = p_boot->clusters_per_record >= 0 ?
            p_boot->clusters_per_record * cluster_size :
            1 << -p_boot->clusters_per_record;
    total_cluster = p_boot->total_sectors_64 / p_boot->sectors_per_cluster;
    MFTDump();
}


// Read and return a pointer to the data block of the record
// Input start sector
uint8_t* NTFS_Reader::readMFTRecord(uint64_t start) {
    uint8_t* buffer = new uint8_t[record_size];
    readSector(start, 2, buffer);
    return buffer;
}


uint8_t* NTFS_Reader::MFTDump() {
    dir.push(5);
    uint64_t record_number;
    uint64_t cur_sector = p_boot->sectors_per_cluster * p_boot->MFT_cluster;

    // REad the MFT
    uint8_t* mftfile = readMFTRecord(cur_sector);

    pRecord fileRecord = (pRecord) mftfile;
    Attr_Header *attribute = (Attr_Header *) (mftfile + fileRecord->attributes_offset);
    Attr_Header_NR *dataAttribute = nullptr;
    while (true) {
        if (attribute->type == 0x80) {
            dataAttribute = (Attr_Header_NR *) attribute;
        } else if (attribute->type == 0xFFFFFFFF) {
            break;
        }
        attribute = (Attr_Header *) ((uint8_t *) attribute + attribute->length);
    }
    RunHeader *dataRun = (RunHeader *) ((uint8_t *) dataAttribute + dataAttribute->run_list_offset);
    uint64_t clusterNumber = 0;
    while (((uint8_t *) dataRun - (uint8_t *) dataAttribute) < dataAttribute->length && dataRun->lengthFieldBytes) {
        uint64_t length = 0, offset = 0;
        for (int i = 0; i < dataRun->lengthFieldBytes; i++) {
            length |= (uint64_t) (((uint8_t *) dataRun)[1 + i]) << (i * 8);
        }

        for (int i = 0; i < dataRun->offsetFieldBytes; i++) {
            offset |= (uint64_t) (((uint8_t *) dataRun)[1 + dataRun->lengthFieldBytes + i]) << (i * 8);
        }

        if (offset & ((uint64_t) 1 << (dataRun->offsetFieldBytes * 8 - 1))) {
            // Sign extend the offset.

            for (int i = dataRun->offsetFieldBytes; i < 8; i++) {
                offset |= (uint64_t) 0xFF << (i * 8);
            }
        }
        clusterNumber += offset;
        dataRun = (RunHeader *) ((uint8_t *) dataRun + 1 + dataRun->lengthFieldBytes + dataRun->offsetFieldBytes);
        uint64_t filesRemaining = length * cluster_size / record_size;
        uint64_t positionInBlock = 0;

        while (filesRemaining) {
            uint64_t filesToLoad = record_size;
            if (filesRemaining < MFT_FILES_PER_BUFFER) filesToLoad = filesRemaining;
//            std::wcout << clusterNumber * cluster_size + positionInBlock << '\n';
            readSector(clusterNumber * p_boot->sectors_per_cluster + positionInBlock / sector_size, filesToLoad * MFT_FILE_SIZE / sector_size, mftBuffer);
            filesRemaining -= filesToLoad;

            uint64_t pos_offset = clusterNumber * p_boot->sectors_per_cluster + positionInBlock / sector_size;
            for (int i = 0; i < filesToLoad; i++, pos_offset += record_size / sector_size) {
                // Process the file record.
                pRecord fileRecord = (pRecord) (mftBuffer + MFT_FILE_SIZE * i);
                if (memcmp(fileRecord->type, "FILE", 4) != 0) {
                    continue;
                }
                if (!fileRecord->flag == 0x01) continue;
                Attr_Header *attribute = (Attr_Header *) ((uint8_t *) fileRecord + fileRecord->attributes_offset);
                FileS* last_file;
                while ((uint8_t *) attribute - (uint8_t *) fileRecord < MFT_FILE_SIZE) {
                    if (attribute->type == 0x30) {
                        // Parse the $FILE_NAME attribute.
                        FileNameAttributeHeader *fileNameAttribute = (FileNameAttributeHeader *) attribute;
                        if (fileNameAttribute->namespaceType != 2 && !fileNameAttribute->form_code) {

                            FileS file = {};
                            file.start_sector = pos_offset;
                            file.size = fileNameAttribute->realSize;
                            file.parent = fileNameAttribute->parentRecordNumber;
                            file.flag = fileNameAttribute->flags;
                            file.record_number = fileRecord->record_number;

                            wchar_t* tmp = &fileNameAttribute->fileName[0];
                            wchar_t* tmp2 = new wchar_t[fileNameAttribute->fileNameLength];
                            memcpy(tmp2, tmp, fileNameAttribute->fileNameLength * 2);
                            for (int i = 0; i < fileNameAttribute->fileNameLength; ++i) {
                                file.name.push_back(tmp2[i]);
                            }
                            delete[] tmp2;
                            v_items[fileNameAttribute->parentRecordNumber].push_back(file);
                            last_file = &v_items[fileNameAttribute->parentRecordNumber].back();
                            wchar_t *f_name = &fileNameAttribute->fileName[1];
                        }
                    }
                    if (attribute->type == 0x80) {
                        if (attribute->form_code == 0x01) {
                            Attr_Header_NR* data = (Attr_Header_NR*) attribute;
                            last_file->size = data->content_disk_size;

                            } else {
                            Attr_Header_R* data = (Attr_Header_R*) attribute;
                            last_file->size = data->content_length;
                        }

                    }
                    if (attribute->type == 0xFFFFFFFF) {
                        break;
                    }
                    attribute = (Attr_Header *) ((uint8_t *) attribute + attribute->length);
                }
            }
            positionInBlock += filesToLoad * record_size;
        }
    }
}