//
// Created by user on 6/24/2021.
//

#include "Filesystem_Reader.h"

void printStr(std::wstring &ws, const char *s, int l, int num) {
    for (int i = l; i < l + num; ++i) {
        if (s[i] == ' ') {
            num = i - l + 1;
            break;
        }
    }
    auto* wc = new wchar_t[num + 1];
    mbstowcs(wc, s + l, num);
    wc[num] = 0x00;
    ws.append(wc);
}

Filesystem_Reader::Filesystem_Reader(HANDLE device) {
    this->device = device;
}
