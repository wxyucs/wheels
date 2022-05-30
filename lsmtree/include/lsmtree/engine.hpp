#pragma once

#include "lsmtree/memtable.hpp"
#include <map>
#include <string>

namespace lsmtree {

class Engine {
public:
    Engine(const std::string &path) : path_(path) {
        memtable_ = new MemTable();
    }

    std::string Get(const std::string &key) {
        if (memtableIndex_.find(key) == memtableIndex_.end()) {
            return "";
        }
        int32_t offset = memtableIndex_[key];
        char *buffer = memtable_->GetBuffer();
        buffer += offset;
        int32_t *length = reinterpret_cast<int32_t *>(buffer);
        assert(*length > 0);
        buffer += sizeof(int32_t);

        return std::string(buffer, size_t(*length));
    }

    void Set(const std::string &key, const std::string &value) {
        if (not memtable_->CheckSpaceEnough(value)) {
            memtable_ = new MemTable();
        }
        assert(memtable_->CheckSpaceEnough(value));
        int32_t offset = memtable_->AddItem(value);
        memtableIndex_[key] = offset;
    }

private:
    std::string path_;
    MemTable *memtable_;
    std::map<std::string, int32_t> memtableIndex_;
};

} // namespace lsmtree
