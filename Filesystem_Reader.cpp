//
// Created by user on 6/24/2021.
//

#include "Filesystem_Reader.h"

void printStr(std::wstring &ws, const char *s, int l, int num) {
    for (int i = l; i < l + num; ++i) {
        if (s[i] == ' ') {
            num = i - l;
            break;
        }
    }
    auto* wc = new wchar_t[num + 1];
    wc[num] = L'\0';
    mbstowcs(wc, s + l, num + 1);
    for (int i = 0; i < num; ++i) {
        ws.push_back(wc[i]);
    }
}


Filesystem_Reader::Filesystem_Reader(HANDLE device) {
    this->device = device;
}


