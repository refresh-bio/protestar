#pragma once

#include <vector>
#include <string>
#include "struct-compressor.h"
#include "../parsers/cif.h"

// *****************************************************************
class CifCompressor : public StructCompressor
{
    const vector<string> v_words = {
        "_entry.id", "_atom_type", "_chem_comp", "_citation", "_entity", "_entity_poly_seq", "_pdbx_data_usage", "_struct_ref_seq", "_atom_site", 
        "_audit_author", "_audit_conform", "_citation_author", "_entity_poly", "_entity_poly_seq", "_ma_data", "_ma_model_list", "_ma_protocol_step", 
        "_ma_qa_metric", "_ma_qa_metric_global", "_ma_qa_metric_local", "_ma_software_group", "_ma_target_entity", "_ma_target_entity_instance",
        "_pdbx_audit_revision_details", "_pdbx_audit_revision_history", "_pdbx_data_usage", "_pdbx_database_status", "_pdbx_poly_seq_scheme",
        "_struct_conf", "_struct_ref_seq", "L-PEPTIDE LINKING", "rot_matrix", "tr_vector", "template",
    };

public:
    CifCompressor(bool compression_mode) : StructCompressor(compression_mode)
    {
        for (const auto& vec : {v_atom, v_chem, v_col_names, v_words, v_common_words})
            for (const auto& x : vec)
            {
                dict_append(zstd_dict, x, 0);
            }

        zstd_dict_ext = zstd_dict;
    }

	~CifCompressor() = default;
};

// EOF
