#pragma once

#include "entries.h"
#include "input.h"

#include <functional>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>

/******************************************************************/
class Cif : public StructFile {
    
public:

    static constexpr const char* ENTRY_ATOM_SITE{ "_atom_site" };

    const char* getAtomEntryName() const override { return ENTRY_ATOM_SITE; }

    const std::unordered_set<std::string> MINIMAL_SECTIONS
    {
        "_entry",
        "_atom_type",
        "_atom_site"
    };

    Cif(bool minimal_mode, size_t bufSize = 1 << 24) : StructFile(minimal_mode, bufSize)
    {
    }

    void parse() override;

    size_t store() override;
};
