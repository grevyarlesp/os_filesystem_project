//
// Created by user on 6/26/2021.
//

#ifndef FILESYSTEM_VIEWER_ITEM_H
#define FILESYSTEM_VIEWER_ITEM_H

#include <string>
#include <utility>


class Item {
protected:
    std::wstring name;
    bool is_directory; // 0 for folder, 1 for file
    unsigned int first_cluster;
public:
    virtual uint32_t getSize() = 0;
    virtual std::wstring getName();
    virtual bool isDirectory() {
        return is_directory;
    }
    virtual uint32_t getFistCluster() {
        return first_cluster;
    }

};

class File : public Item {
private:
    uint32_t  size = 0;
public:
    File(std::wstring ws, unsigned int first_cluster, uint32_t size) {
        name = std::move(ws);
        is_directory = false;
        this->first_cluster = first_cluster;
        this->size = size;
    };
    uint32_t getSize() override;
};

class Directory : public Item {
public:
    Directory(std::wstring ws, unsigned int first_cluster)  {
        name = std::move(ws);
        is_directory = true;
        this->first_cluster = first_cluster;
    };
    uint32_t getSize() override;
};


#endif //FILESYSTEM_VIEWER_ITEM_H

