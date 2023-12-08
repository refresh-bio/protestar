#pragma once

#include "entries.h"
#include "input-load.h"
#include "cif-base.h"

#include <functional>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>

/******************************************************************/
class CifInput : public StructFileInput, public CifBase
{

public:
    CifInput(bool minimal_mode, size_t bufSize = 1 << 24) : StructFileInput(minimal_mode, bufSize)
    {
    }

    void parse();
};
