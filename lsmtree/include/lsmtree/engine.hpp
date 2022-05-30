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
        std::string filename = AbsolutePath(metaFilename_);
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
            FlushMemTable();
            memtable_ = new MemTable(segmentSize_);
        }
        assert(memtable_->CheckSpaceEnough(value));
        int32_t offset = memtable_->AddItem(value);
        memtableIndex_[key] = offset;
    }

private:
    std::string AbsolutePath(const std::string &filename) {
        return path_ + "/" + filename;
    }

    void FlushMemTable() {
        std::string filename = 
            AbsolutePath(std::to_string(numSegments_) + datafilePrefix_);
        FILE *pFile = fopen(filename.c_str(), "w");
        assert(pFile);
        fwrite(memtable_->GetBuffer(), 
               sizeof(char), 
               size_t(memtable_->GetCapcity()), 
               pFile);
        fclose(pFile);

        filename = AbsolutePath(std::to_string(numSegments_) + mapfilePrefix_);
        pFile = fopen(filename.c_str(), "w");
        assert(pFile);
        assert(pFile);
        for (auto &kv : memtableIndex_) {
            std::string line = kv.first + "," 
                + std::to_string(kv.second) + "\n";
            fputs(line.c_str(), pFile);
        }
        fclose(pFile);

        ++numSegments_;
        filename = AbsolutePath(metaFilename_);
        pFile = fopen(filename.c_str(), "w");
        assert(pFile);
        fwrite(&numSegments_, sizeof(numSegments_), 1, pFile);
        fclose(pFile);
    }

    const std::string metaFilename_ = "engine.meta";
    const std::string datafilePrefix_ = ".data";
    const std::string mapfilePrefix_ = ".map";
    std::string path_;
    int32_t segmentSize_;
    int32_t numSegments_;
    MemTable *memtable_;
    std::map<std::string, int32_t> memtableIndex_;
};

} // namespace lsmtree
