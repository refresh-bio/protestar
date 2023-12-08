#pragma once
#include "../compressors/ext-dict-base.h"

class ExtDictCif : public ExtDictBase
{
protected:
    const vector<string> v_words = {
        "_entry.id", "_atom_type", "_chem_comp", "_citation", "_entity", "_entity_poly_seq", "_pdbx_data_usage", "_struct_ref_seq", "_atom_site",
        "_audit_author", "_audit_conform", "_citation_author", "_entity_poly", "_entity_poly_seq", "_ma_data", "_ma_model_list", "_ma_protocol_step",
        "_ma_qa_metric", "_ma_qa_metric_global", "_ma_qa_metric_local", "_ma_software_group", "_ma_target_entity", "_ma_target_entity_instance",
        "_pdbx_audit_revision_details", "_pdbx_audit_revision_history", "_pdbx_data_usage", "_pdbx_database_status", "_pdbx_poly_seq_scheme",
        "_struct_conf", "_struct_ref_seq", "L-PEPTIDE LINKING", "rot_matrix", "tr_vector", "template",
    };

public:
    ExtDictCif() = default;

    void init_type_specific_dict(const vector<string> &v_atom, const vector<string> &v_chem, const vector<string> &v_col_names, const vector<string> &v_common_words)
    {
        for (const auto& vec : { v_atom, v_chem, v_col_names, v_words, v_common_words })
            for (const auto& x : vec)
            {
                dict_append(type_specific_dict, x, 0);
            }
    }
};