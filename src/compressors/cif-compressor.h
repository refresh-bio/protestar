#pragma once

#include <vector>
#include <string>
#include "struct-compressor.h"
#include "ext-dict-cif.h"

// *****************************************************************
class CifCompressor : public StructCompressor, public ExtDictCif
{
public:
    CifCompressor() : StructCompressor()
    {
        init_type_specific_dict(v_atom, v_chem, v_col_names, v_common_words);

        zstd_dict = type_specific_dict;
        zstd_dict_ext = type_specific_dict;
    }

	~CifCompressor() = default;
};

// EOF
