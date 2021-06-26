//
// Created by user on 6/26/2021.
//

#include "Item.h"

uint32_t File::getSize() {
    return this->size;
}

uint32_t Directory::getSize() {
    return 0;
}

std::wstring Item::getName() {
    return this->name;
}
