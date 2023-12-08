#pragma once

#include <array>
#include <vector>
#include <cinttypes>

#include "../libs/refresh/range_coder/rc.h"
#include "../common/atom_extractor.h"
#include "../core/utils.h"
#include "../parsers/input-base.h"
#include "model_compress.h"
#include "serializer.h"
#include <zstd.h>
#include <vectorclass.h>

using namespace std;
using namespace refresh;

// *****************************************************************
class StructBase
{
public:
    enum class col_type_t { text = 0, cart = 1, numeric_general = 2, numeric_same_values = 3, numeric_same_diff = 4, cart_as_numeric = 5 };

protected:
    const int zstd_compression_level = 19;
    const int cart_working_precision = 6;

    using context_t = uint64_t;

    //    static const uint32_t prefix_max_bits = 13;                     // !!! max: 15        
    static const uint32_t prefix_max_bits = 13;                     // !!! max: 15        
    //    static const uint32_t prefix_short_max_bits = 4;
    static const uint32_t prefix_short_max_bits = 5;
    //    static const uint32_t prefix_model_size = 2 * prefix_max_bits + 3;
    static const uint32_t prefix_model_size = prefix_max_bits + 2;

    const vector<string> v_atom{ "N", "C", "CA", "O", "CB", "CG", "CG1", "CG2", "CD", "CD1", "CD2", "CE", "CE1", "CE2", "CE3", "CZ", "CZ2", "CZ3", "CH2",
    "ND1", "ND2", "NE", "NE1", "NE2", "NH1", "NH2", "NZ", "OG", "OG1", "OD1", "OD2", "OE1", "OE2", "OH", "OXT", "SG", "SD", "SE" };

    const vector<string> v_chem{
        "\"C3 H7 N O2\"" , "ALA", "ALANINE",
        "\"C6 H15 N4 O2\"" , "ARG", "ARGININE",
        "\"C4 H8 N2 O3\"", "ASN", "ASPARAGINE",
        "\"C4 H7 N O4\"",  "ASP", "\"ASPARTIC ACID\"",
        "\"C3 H7 N O2 S\"", "CYS", "CYSTEINE",
        "\"C5 H10 N2 O3\"", "GLN", "GLUTAMINE"
        "\"C5 H9 N O4\"", "GLU", "\"GLUTAMIC ACID\"",
        "\"C2 H5 N O2\"", "GLY", "GLYCINE",
        "\"C6 H10 N3 O2\"", "HIS", "HISTIDINE",
        "\"C6 H13 N O2\"", "ILE", "ISOLEUCINE",
        "\"C6 H13 N O2\"", "LEU", "LEUCINE",
        "\"C6 H15 N2 O2\"", "LYS", "LYSINE",
        "\"C5 H11 N O2 S\"", "MET", "METHIONINE",
        "\"C9 H11 N O2\"", "PHE", "PHENYLALANINE",
        "\"C5 H9 N O2\"", "PRO", "PROLINE",
        "\"C3 H7 N O3\"", "SER", "SERINE",
        "\"C4 H9 N O3\"", "THR", "THREONINE",
        "\"C11 H12 N2 O2\"", "TRP", "TRYPTOPHAN",
        "\"C9 H11 N O3\"", "TYR", "TYROSINE",
        "\"C5 H11 N O2\"", "VAL", "VALINE" };

    const vector<string> v_col_names{
        "group_PDB",
        "id",
        "auth_atom_id",
        "label_alt_id",
        "auth_comp_id",
        "auth_asym_id",
        "auth_seq_id",
        "pdbx_PDB_ins_code",
        "Cartn_x",
        "Cartn_y",
        "Cartn_z",
        "occupancy",
        "B_iso_or_equiv",
        "segment",
        "type_symbol",
        "pdbx_formal_charge"
    };

    const vector<string> v_common_words{
        "ESMFOLD", "PREDICTION", "ALPHAFOLD", "MONOMER"
    };

    const set<string> chain_desc_col
    {
        StructFileBase::Columns::group_PDB, StructFileBase::Columns::id, StructFileBase::Columns::auth_atom_id, StructFileBase::Columns::auth_comp_id, StructFileBase::Columns::auth_asym_id,
        StructFileBase::Columns::auth_seq_id, StructFileBase::Columns::Cartn_x, StructFileBase::Columns::Cartn_y, StructFileBase::Columns::Cartn_z, StructFileBase::Columns::B_iso_or_equiv, StructFileBase::Columns::type_symbol
    };

    const set<string> chain_desc_col_string
    {
        StructFileBase::Columns::group_PDB, StructFileBase::Columns::auth_atom_id, StructFileBase::Columns::auth_comp_id, StructFileBase::Columns::auth_asym_id, StructFileBase::Columns::type_symbol
    };

    const set<string> chain_desc_col_numeric
    {
        StructFileBase::Columns::id, StructFileBase::Columns::auth_seq_id, StructFileBase::Columns::Cartn_x, StructFileBase::Columns::Cartn_y, StructFileBase::Columns::Cartn_z, StructFileBase::Columns::B_iso_or_equiv
    };

    const set<string> cartn_desc_col
    {
        StructFileBase::Columns::Cartn_x, StructFileBase::Columns::Cartn_y, StructFileBase::Columns::Cartn_z
    };

    using model_prefix_t = rc_simple_fixed<vector_io_stream, prefix_model_size, 1 << 15, 8>;
    using model_negative_t = rc_simple_fixed<vector_io_stream, 2, 1 << 15, 1>;                  // unused at the current version
    using model_suffix_t = rc_simple<vector_io_stream, 1 << 12, 1>;
    using model_centroid_id_t = rc_simple<vector_io_stream, 1 << 10, 8>;
    using model_tetrahedron_first_t = rc_simple_fixed<vector_io_stream, 2, 1 << 12, 2>;

    uint32_t model_ok_init[2] = { 1, 64 };
    using model_ok_t = rc_simple_fixed<vector_io_stream, 2, 1 << 15, 8>;

    using model_entry_type_t = rc_simple_fixed<vector_io_stream, 5, 1 << 12, 2>;
    using model_col_type_t = rc_simple_fixed<vector_io_stream, 6, 1 << 12, 4>;
    using model_aa_t = rc_simple_fixed<vector_io_stream, 22, 1 << 13, 1>;

    enum class entry_type_t { block = 0, loop_generic = 1, loop_atom = 2, loop_hetatm = 3, eod = 4 };

    // Cartesian conversion-relarted values
    int64_t max_error_backbone_uA_single_axis;
    int64_t max_error_sidechain_uA_single_axis;
    int64_t res_backbone_uA_single_axis = 0;
    int64_t res_sidechain_uA_single_axis = 0;

    bool lossless;
    bool max_compression;
    bool single_bf;
    int cart_precision = 0;
    int64_t cart_working_multiplier = 0;

    vector<char> zstd_dict;
    vector<char> chain_dict;
    vector<char> zstd_dict_ext;

    enum class rc_int_class_t
    {
        no_columns = 0, no_rows = 1, col_decimals = 2, col_width = 3, block_entry_size = 4, numeric_val = 5, numeric_diff = 6,
        aa_sequence_len = 7, bf_delta = 8, string_rep = 9
    };

    rc_context_vec_emb<model_prefix_t> dict_cart_prefix;
    rc_context_vec_emb<model_negative_t> dict_cart_negative;
    rc_context_vec<model_suffix_t> dict_cart_suffix;
    rc_context_vec_emb<model_prefix_t> dict_main_prefix;
    rc_context_vec_emb<model_negative_t> dict_main_negative;
    rc_context_vec<model_suffix_t> dict_main_suffix;
    rc_context_vec<model_centroid_id_t> dict_centroid_id;
    rc_context_vec<model_tetrahedron_first_t> dict_tetrahedron_first;

    rc_context_vec<model_entry_type_t> dict_entry_type;
    rc_context_vec<model_col_type_t> dict_col_type;

    context_t ctx_entry_type;

    vector<uint8_t> v_rc;
    vector_io_stream* vios = nullptr;

    Serializer ser_strings;         // column names, etc
    Serializer ser_text;            // textual data - columns, block entries
    Serializer ser_rc;              // numbers, flags, etc.
    Serializer ser_plain;           // just to gather ser_strings + ser_text for ZSTD compression

    Serializer ser_full;

    Protein protein;

    map<const LoopEntry*, Protein> loop_atom_sections;
    vector<aa_t> aa_sequences;
    vector<aa_t>::iterator aa_sequences_iter;

    void clear_serializers();

    void extend_zstd_dict(const string& struct_name);
    void add_aa_codes_to_dict(const vector<chain_desc_t>& chains);

    void dict_append(vector<char>& dict, const string& str, char sep)
    {
        dict.insert(dict.end(), str.begin(), str.end());
        dict.push_back(sep);
    }

    void dict_append(string& dict, const string& str)
    {
        dict.insert(dict.end(), str.begin(), str.end());
    }

    template<typename T> uint32_t no_bits_max15(T x)
    {
        if (x == 0)
            return 0;
        else if (x < 2)
            return 1;
        else if (x < 4)
            return 2;
        else if (x < 8)
            return 3;
        else if (x < 16)
            return 4;
        else if (x < 32)
            return 5;
        else if (x < 64)
            return 6;
        else if (x < 128)
            return 7;
        else if (x < 256)
            return 8;
        else if (x < 512)
            return 9;
        else if (x < 1024)
            return 10;
        else if (x < 2048)
            return 11;
        else if (x < 4096)
            return 12;
        else if (x < 8192)
            return 13;
        else if (x < 16384)
            return 14;
        else
            return 15;
    }

    template<typename T> uint32_t no_bits(T x)
    {
        uint32_t r;

        for (r = 0; x; ++r)
            x >>= 1;

        return r;
    }

    template<typename T> void delete_var(T*& var)
    {
        if (var)
        {
            delete var;
            var = nullptr;
        }
    }

    void clear_rc_dict();

public:
    StructBase() :
        max_error_backbone_uA_single_axis(0),
        max_error_sidechain_uA_single_axis(0),
        lossless(true),
        max_compression(true),
        single_bf(false)
    {}

    ~StructBase()
    {}
};

// EOF
