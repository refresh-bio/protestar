#pragma once

#include "struct-compressor.h"
#include "ext-dict-pdb.h"

//#define SHOW_CIF_COMPRESSOR_STATS

// *****************************************************************
class PDBCompressor : public StructCompressor, public ExtDictPdb
{
public:
	PDBCompressor() : StructCompressor()
	{
		init_type_specific_dict(v_atom, v_chem, v_col_names, v_common_words);

		zstd_dict = type_specific_dict;
		zstd_dict_ext = zstd_dict;
	}

	~PDBCompressor() = default;
};

// EOF
