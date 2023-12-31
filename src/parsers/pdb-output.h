#pragma once

#include "entries.h"
#include "input-save.h"
#include "pdb-base.h"

#include <functional>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>

/******************************************************************/
class PdbOutput : public StructFileOutput, public PdbBase
{
public:
    PdbOutput(bool minimal_mode, size_t bufSize = 1 << 24) : StructFileOutput(minimal_mode, bufSize)
    {
    }

    size_t store();
};
