//
// Created by user on 6/26/2021.
//

#include "Item.h"

uint32_t File::getSize() {
    return this->size;
}

std::wstring Item::getAttribute() {
    switch (this->attrib) {
        case 0x01:
            return L"READ_ONLY";
            break;
        case 0x02:
            return L"HIDDEN";
            break;
        case 0x04:
            return L"SYSTEM";
            break;
        case 0x08:
            return L"VOLUME_ID";
            break;
        case 0x10:
            return L"DIRECTORY";
            break;
        case 0x20:
            return L"ARCHIVE";
            break;
    }
    return L"No attribute";
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

Directory::~Directory() = default;

std::wstring Item::getName() {
    return this->name;
}
