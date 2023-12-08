#pragma once

#include <vector>
#include <string>
#include "struct-decompressor.h"
#include "ext-dict-cif.h"

// *****************************************************************
class CifDecompressor : public StructDecompressor, public ExtDictCif
{
public:
    CifDecompressor() : StructDecompressor()
    {
        init_type_specific_dict(v_atom, v_chem, v_col_names, v_common_words);

        zstd_dict = type_specific_dict;
        zstd_dict_ext = type_specific_dict;
    }

    ~CifDecompressor() = default;
};

// EOF
