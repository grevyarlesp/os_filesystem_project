//
// Created by user on 6/24/2021.
//

#include "Filesystem_Reader.h"

void printStr(const char *s, int l, int num) {
    wchar_t* wc = new wchar_t[num];
    mbstowcs (wc, s + l, num);
    for (int i = 0; i < num; ++i)
        std::wcout << wc[i];
}

Filesystem_Reader::Filesystem_Reader(HANDLE device) {
    this->device = device;
}
