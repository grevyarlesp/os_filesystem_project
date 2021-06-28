//
// Created by user on 6/26/2021.
//

#include "Item.h"

uint32_t File::getSize() {
    return this->size;
}

std::string File::getExt() {
    return ext;
}

std::wstring Item::getAttribute() {
    std::wstring ans;
    if (this->attrib & 0x01)
        ans += L"READ_ONLY ";
    if (this->attrib & 0x02)
        ans += L"HIDDEN ";
    if (this->attrib & 0x04)
        ans += L"SYSTEM ";
    if (this->attrib & 0x08)
        ans += L"VOLUME_ID ";
    if (this->attrib & 0x10)
        ans += L"DIRECTORY ";
    if (this->attrib & 0x20)
        ans += L"ARCHIVE ";
    return ans;
}

File::~File() = default;

uint32_t Directory::getSize() {
    return 0;
}

Directory::Directory(std::wstring ws, unsigned int first_cluster, uint8_t attrib) {
    name = std::move(ws);
    is_directory = true;
    this->first_cluster = first_cluster;
    this->attrib = attrib;
}

std::string Directory::getExt() {
    return "";
}

Directory::~Directory() = default;

std::wstring Item::getName() {
    return this->name;
}

