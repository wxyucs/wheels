#pragma once

#include "lsmtree/memtable.hpp"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace lsmtree {

class Engine {
public:
    Engine(const std::string &path, int32_t segmentSize) 
        : path_(path), segmentSize_(segmentSize), deactiveMaps_(), 
        activeMap_() {
        std::string filename = AbsolutePath(metaFilename_);
        if (std::filesystem::exists(filename)) {
            FILE *pFile = fopen(filename.c_str(), "r");
            fread(&numSegments_, sizeof(int32_t), 1, pFile);
            deactiveMaps_.resize(size_t(numSegments_));
            for (int32_t i = 0; i < numSegments_; ++i) {
                filename = AbsolutePath(std::to_string(i) + mapfilePrefix_);
                std::ifstream ifile(filename);
                std::string line;
                while (std::getline(ifile, line)) {
                    size_t pos = line.find(",");
                    if (pos == std::string::npos or
                            pos == line.size() - 1) {
                        continue;
                    }
                    std::string key = line.substr(0, pos);
                    int32_t offset = std::atoi(line.substr(pos + 1).c_str());
                    deactiveMaps_[size_t(i)][key] = offset;
                }
            }
        } else {
            numSegments_ = 0;
        }
        memtable_ = new MemTable(segmentSize);
    }

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    ~Engine() {
        FlushMemTable();
    }

    std::string Get(const std::string &key) {
        if (activeMap_.find(key) != activeMap_.end()) {
            int32_t offset = activeMap_[key];
            char *buffer = memtable_->GetBuffer();
            buffer += offset;
            int32_t *length = reinterpret_cast<int32_t *>(buffer);
            assert(*length > 0);
            buffer += sizeof(int32_t);

            return std::string(buffer, size_t(*length));
        }

        for (size_t i = 0; i < deactiveMaps_.size(); ++i) {
            std::map<std::string, int32_t> &m = deactiveMaps_[i];
            if (m.find(key) == m.end()) {
                continue;
            }

            int32_t offset = m[key];
            char *buffer =
                reinterpret_cast<char*>(malloc(size_t(segmentSize_)));
            std::string filename =
                AbsolutePath(std::to_string(i) + datafilePrefix_);
            FILE *pFile = fopen(filename.c_str(), "r");
            fread(buffer, sizeof(char), size_t(segmentSize_), pFile);
            int32_t *length = reinterpret_cast<int32_t *>(buffer + offset);
            assert(*length > 0);

            std::string val(buffer + offset + sizeof(int32_t), 
                    size_t(*length));
            free(buffer);
            return val;
        }
        
        return "";
    }

    void Set(const std::string &key, const std::string &value) {
        if (not memtable_->CheckSpaceEnough(value)) {
            FlushMemTable();
            deactiveMaps_.push_back(activeMap_);
            activeMap_.clear();
            memtable_ = new MemTable(segmentSize_);
        }
        assert(memtable_->CheckSpaceEnough(value));
        int32_t offset = memtable_->AddItem(value);
        activeMap_[key] = offset;
    }

    std::vector<std::string> Keys() {
        std::set<std::string> keys;
        for (auto &m : deactiveMaps_) {
            for (auto &kv : m) {
                keys.insert(kv.first);
            }
        }

        return std::vector<std::string>{keys.begin(), keys.end()};
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
        for (auto &kv : activeMap_) {
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
    int32_t segmentSize_ = 0;
    int32_t numSegments_ = 0;
    std::vector<std::map<std::string, int32_t>> deactiveMaps_;
    std::map<std::string, int32_t> activeMap_;
    MemTable *memtable_ = nullptr;
};

} // namespace lsmtree
