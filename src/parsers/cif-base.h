#pragma once

#include <unordered_set>
#include <string>

class CifBase
{
public:
    static constexpr const char* ENTRY_ATOM_SITE{ "_atom_site" };

    //    const char* getAtomEntryName() const override { return ENTRY_ATOM_SITE; }

    const std::unordered_set<std::string> MINIMAL_SECTIONS
    {
        "_entry",
        "_atom_type",
        "_atom_site"
    };

    CifBase() = default;
};
