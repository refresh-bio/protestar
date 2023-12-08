#pragma once
#include "../parsers/input-base.h"

class StructFileInput : public StructFileBase
{
public:
    StructFileInput(bool minimal_mode, size_t bufSize = 1 << 24) :
        StructFileBase(minimal_mode, bufSize)
    {}

    virtual ~StructFileInput() {};

    virtual size_t load(const std::string& fileName);
    virtual size_t load(const std::vector<uint8_t>& file_data);
};