#pragma once

#include "struct-decompressor.h"
#include "ext-dict-pdb.h"

// *****************************************************************
class PDBDecompressor : public StructDecompressor, public ExtDictPdb
{
public:
	PDBDecompressor() : StructDecompressor()
	{
		init_type_specific_dict(v_atom, v_chem, v_col_names, v_common_words);

		zstd_dict = type_specific_dict;

		zstd_dict_ext = zstd_dict;
	}

	~PDBDecompressor() = default;
};

// EOF
