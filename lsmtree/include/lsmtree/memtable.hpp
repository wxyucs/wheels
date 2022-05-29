#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

namespace lsmtree {

class MemTable {
public:
    MemTable(int32_t capcity = 4ll * 1024 * 1024) 
        : capcity_(capcity), free_(capcity - 1) {
        buffer_ = reinterpret_cast<char *>(malloc(size_t(capcity_)));
        memset(buffer_, 0, size_t(capcity_));
        assert(buffer_);

        pNumItems_ = reinterpret_cast<int32_t *>(buffer_);
        offsets_ = pNumItems_ + 1;
    }

    int32_t AddItem(const std::string &value) {
        int32_t endOff = capcity_;
        if (*pNumItems_ != 0) {
            endOff = offsets_[*pNumItems_ - 1];
        }

        int32_t itemSize = int32_t(sizeof(int32_t) + value.length());
        int32_t startOff = endOff - itemSize;
        int32_t *pItem = reinterpret_cast<int32_t *>(buffer_ + startOff);
        *pItem = int32_t(value.size());
        
        memcpy(reinterpret_cast<char *>(pItem + 1), value.data(), value.size());

        offsets_[*pNumItems_] = startOff;
        *pNumItems_ = *pNumItems_ + 1;
        free_ -= int32_t(sizeof(int32_t)) + itemSize;

        return startOff;
    }

    bool CheckSpaceEnough(const std::string &value) {
        return free_ > int32_t(sizeof(int32_t) + sizeof(int32_t) + value.length());
    }

    const int32_t capcity_;
    int32_t free_;
    char *buffer_ = nullptr;
    int32_t *pNumItems_ = nullptr;
    int32_t *offsets_ = nullptr;
};

} // namespace lsmtree
