#include "struct-compressor.h"

#include <iterator>

// *****************************************************************
void StructBase::clear_rc_dict()
{
    dict_cart_prefix.clear();
    dict_cart_negative.clear();
    dict_cart_suffix.clear();
    dict_main_prefix.clear();
    dict_main_negative.clear();
    dict_main_suffix.clear();
    dict_centroid_id.clear();
    dict_tetrahedron_first.clear();

    dict_entry_type.clear();
    ctx_entry_type = 0;
    dict_col_type.clear();

    v_rc.clear();
    vios->restart_read();
}

// *****************************************************************
void StructBase::add_aa_codes_to_dict(const vector<chain_desc_t>& chains)
{
    chain_dict.clear();

    string one_let;
    string three_let_0;
    string three_let_sp;

    for (const auto& chain : chains)
    {
        for (const auto& aa : chain.aa)
        {
            one_let.push_back(aa_type_char(aa.type));
            dict_append(three_let_0, aa_to_str(aa.type));
            three_let_0.push_back(0);
            dict_append(three_let_sp, aa_to_str(aa.type));
            three_let_sp.push_back(' ');
        }
    }

    dict_append(chain_dict, one_let, 0);
    dict_append(chain_dict, three_let_0, 0);
    dict_append(chain_dict, three_let_sp, 0);
}

// *****************************************************************
void StructBase::clear_serializers()
{
    ser_plain.clear();
    ser_strings.clear();
    ser_text.clear();
    ser_rc.clear();
    ser_full.clear();
}

// *****************************************************************
void StructBase::extend_zstd_dict(const string& struct_name)
{
    // Remove any extra data from previous compressions/decompressions
    zstd_dict_ext.resize(zstd_dict.size());

    zstd_dict_ext.insert(zstd_dict_ext.end(), struct_name.begin(), struct_name.end());

    for (const auto x : aa_sequences)
        zstd_dict_ext.emplace_back(aa_type_char(x));

    for (const auto x : aa_sequences)
        dict_append(zstd_dict_ext, aa_to_str(x), 0);
    for (const auto x : aa_sequences)
        dict_append(zstd_dict_ext, aa_to_str(x), ' ');
}

// EOF
