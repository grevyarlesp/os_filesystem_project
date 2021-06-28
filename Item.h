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
    uint8_t attrib;
public:
    virtual uint32_t getSize() = 0;
    virtual std::wstring getName();
    virtual bool isDirectory() {
        return is_directory;
    }
    virtual uint32_t getFirstCluster() {
        return first_cluster;
    }
    virtual std::wstring getAttribute();

    virtual std::string getExt()=0;
};

class File : public Item {
private:
    uint32_t  size = 0;
    std::string ext;
public:
    File(std::wstring ws, unsigned int first_cluster, uint32_t size, uint8_t attrib, std::string ext) {
        name = std::move(ws);
        is_directory = false;
        this->first_cluster = first_cluster;
        this->size = size;
        this->attrib = attrib;
        this->ext = std::move(ext);
    };
    uint32_t getSize() override;
    std::string getExt() override;
    ~File();
};

class Directory : public Item {
public:
    Directory(std::wstring ws, unsigned int first_cluster, uint8_t attrib);
    uint32_t getSize() override;
    ~Directory();
    std::string getExt() override;
};


#endif //FILESYSTEM_VIEWER_ITEM_H

