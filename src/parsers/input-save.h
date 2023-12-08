#pragma once
#include "../parsers/input-base.h"

class StructFileOutput : public StructFileBase
{
public:
    StructFileOutput(bool minimal_mode, size_t bufSize = 1 << 24) :
        StructFileBase(minimal_mode, bufSize)
    {}

    virtual ~StructFileOutput() {};

    virtual void save(const std::string& fileName) {
        std::ofstream ofs(fileName, std::ios_base::binary);
        if (ofs) {
            ofs.write(fileBuffer.data(), fileBufferBytes);
        }
    }
};