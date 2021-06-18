#include <windows.h>
#include <stdio.h>
#include <iostream>

typedef struct fat_BS
{
    unsigned char 		bootjmp[3];
    unsigned char 		oem_name[8];
    unsigned short 	        bytes_per_sector;
    unsigned char		sectors_per_cluster;
    unsigned short		reserved_sector_count;
    unsigned char		table_count;
    unsigned short		root_entry_count;
    unsigned short		total_sectors_16;
    unsigned char		media_type;
    unsigned short		table_size_16;
    unsigned short		sectors_per_track;
    unsigned short		head_side_count;
    unsigned int 		hidden_sector_count;
    unsigned int 		total_sectors_32;
    unsigned char		extended_section[54];
    //extended fat32 stuff
    unsigned int		table_size_32;
    unsigned short		extended_flags;
    unsigned short		fat_version;
    unsigned int		root_cluster;
    unsigned short		fat_info;
    unsigned short		backup_BS_sector;
    unsigned char 		reserved_0[12];
    unsigned char		drive_number;
    unsigned char 		reserved_1;
    unsigned char		boot_signature;
    unsigned int 		volume_id;
    unsigned char		volume_label[11];
    unsigned char		fat_type_label[8];
}__attribute__((packed)) *pFat_BS_T;

int ReadSector(const char*  drive, int readPoint, BYTE sector[512])
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,    // Drive to open
                        GENERIC_READ,           // Access mode
                        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
                        NULL,                   // Security Descriptor
                        OPEN_EXISTING,          // How to create
                        0,                      // File attributes
                        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        printf("CreateFile: %u\n", GetLastError());
        return 1;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
    }
    else
    {
        printf("Success!\n");
    }
}

void ReadBootSector() {

}
uint16_t swap_uint16( uint16_t val )
{
    return (val << 8) | (val >> 8 );
}
int main(int argc, char ** argv)
{
    BYTE sector[512];
    ReadSector("\\\\.\\F:",0, sector);
    pFat_BS_T pFat_boot = (pFat_BS_T) sector;
    std::cout << pFat_boot->oem_name << '\n';
    std::cout << pFat_boot->bytes_per_sector << '\n';
    std::cout << (int) pFat_boot->sectors_per_cluster;
    return 0;
}