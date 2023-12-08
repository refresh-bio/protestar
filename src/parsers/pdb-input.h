#pragma once

#include "entries.h"
#include "input-load.h"
#include "pdb-base.h"

#include <functional>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>

/******************************************************************/
class PdbInput : public StructFileInput, public PdbBase
{

public:
    PdbInput(bool minimal_mode, size_t bufSize = 1 << 24) : StructFileInput(minimal_mode, bufSize)
    {
    }

    void parse();
};
