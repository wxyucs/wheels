#pragma once

#include "lsmtree/memtable.hpp"
#include <cstdio>
#include <filesystem>
#include <map>
#include <string>

namespace lsmtree {

class Engine {
public:
    Engine(const std::string &path, int32_t segmentSize) 
        : path_(path), segmentSize_(segmentSize) {
        std::string filename = path + "/" + metaFilename_;
        if (std::filesystem::exists(filename)) {
            FILE *pFile = fopen(filename.c_str(), "r");
            fread(&numSegments_, sizeof(int32_t), 1, pFile);
        } else {
            numSegments_ = 0;
        }
        memtable_ = new MemTable(segmentSize);
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
            std::string filename = path_ + "/" 
                + std::to_string(numSegments_) + ".data";
            FILE *pFile = fopen(filename.c_str(), "w");
            assert(pFile);
            fwrite(memtable_->GetBuffer(), 
                   sizeof(char), 
                   size_t(memtable_->GetCapcity()), 
                   pFile);
            fclose(pFile);
            ++numSegments_;
            filename = path_ + "/" + metaFilename_;
            pFile = fopen(filename.c_str(), "w");
            assert(pFile);
            fwrite(&numSegments_, sizeof(numSegments_), 1, pFile);
            fclose(pFile);

            memtable_ = new MemTable(segmentSize_);
        }
        assert(memtable_->CheckSpaceEnough(value));
        int32_t offset = memtable_->AddItem(value);
        memtableIndex_[key] = offset;
    }

private:
    const std::string metaFilename_ = "engine.meta";
    std::string path_;
    int32_t segmentSize_;
    int32_t numSegments_;
    MemTable *memtable_;
    std::map<std::string, int32_t> memtableIndex_;
};

} // namespace lsmtree
