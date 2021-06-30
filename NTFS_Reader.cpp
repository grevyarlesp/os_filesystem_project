#include "NTFS_Reader.h"

bool NTFS_Reader:: readBootSector() {
    p_boot = (pNTFS_BS_t) malloc(512);
    DWORD bytesRead;
    SetFilePointer(device, 0, nullptr, FILE_BEGIN);
    if (! ReadFile(device, p_boot, 512, &bytesRead, nullptr) && p_boot->end_marker==0xAA55) {
        return false;
    }
    // https://www.cse.scu.edu/~tschwarz/coen252_07Fall/Lectures/NTFS.html
    if (p_boot->clusters_per_record < 0x7F)
        sectors_per_index_buffer = p_boot->clusters_per_record * p_boot->sectors_per_cluster;
    else
        sectors_per_record = p_boot->sectors_per_cluster / uint32_t(1 << (p_boot->clusters_per_record));
    if (p_boot->clusters_per_index < 0x7F)
        sectors_per_index_buffer = p_boot->clusters_per_index * p_boot->sectors_per_cluster;
    else
        sectors_per_record = p_boot->sectors_per_cluster / uint32_t(1 << (p_boot->clusters_per_index));
//    std::wcout << p_boot->clusters_per_index;
//    std::wcout << '\n';
//    std::wcout << p_boot->clusters_per_record;

    return true;
}
bool NTFS_Reader::readSector(unsigned int sectorToRead, int sectorCount, uint8_t *sector) {
    DWORD bytesRead;
    int retCode = 0;
    SetFilePointer(device, (LONG) sectorToRead * p_boot->bytes_per_sector, nullptr, FILE_BEGIN);//Set a Point to Read
    return ReadFile(device, sector, (LONG) sectorCount * p_boot->bytes_per_sector, &bytesRead, nullptr);
}

void NTFS_Reader::printCurrentDirectory() {

}

void NTFS_Reader::openItem(int item_number) {

}

void NTFS_Reader::printBootInformation() {
    std::wcout << sizeof(NTFS_BS)  << '\n';
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
    std::wcout << "Clusters/index: " << p_boot->clusters_per_index << '\n';
}
