#pragma once

#include <array>
#include <vector>
#include <cinttypes>

#include <rc.h>
#include "../common/atom_extractor.h"
#include "../core/utils.h"
#include "model_compress.h"
#include "serializer.h"
#include <zstd.h>
#include <vectorclass.h>

using namespace std;
using namespace refresh;

// *****************************************************************
class StructCompressor
{
public:
    enum class col_type_t { text = 0, cart = 1, numeric_general = 2, numeric_same_values = 3, numeric_same_diff = 4, cart_as_numeric = 5 };

protected:
    const int zstd_compression_level = 19;
    const int cart_working_precision = 6;

    using context_t = uint64_t;

//    static const uint32_t prefix_max_bits = 12;                     // !!! max: 15
    static const uint32_t prefix_max_bits = 13;                     // !!! max: 15        ecoli -11kB
    static const uint32_t prefix_short_max_bits = 4;
//    static const uint32_t prefix_short_max_bits = 5;
    static const uint32_t prefix_model_size = 2 * prefix_max_bits + 3;

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

    const vector<string> v_common_words {
        "ESMFOLD", "PREDICTION", "ALPHAFOLD", "MONOMER"
    };

    const set<string> chain_desc_col
    {
        StructFile::Columns::group_PDB, StructFile::Columns::id, StructFile::Columns::auth_atom_id, StructFile::Columns::auth_comp_id, StructFile::Columns::auth_asym_id,
        StructFile::Columns::auth_seq_id, StructFile::Columns::Cartn_x, StructFile::Columns::Cartn_y, StructFile::Columns::Cartn_z, StructFile::Columns::B_iso_or_equiv, StructFile::Columns::type_symbol
    };

    const set<string> chain_desc_col_string
    {
        StructFile::Columns::group_PDB, StructFile::Columns::auth_atom_id, StructFile::Columns::auth_comp_id, StructFile::Columns::auth_asym_id, StructFile::Columns::type_symbol
    };

    const set<string> chain_desc_col_numeric
    {
        StructFile::Columns::id, StructFile::Columns::auth_seq_id, StructFile::Columns::Cartn_x, StructFile::Columns::Cartn_y, StructFile::Columns::Cartn_z, StructFile::Columns::B_iso_or_equiv
    };

    const set<string> cartn_desc_col
    {
        StructFile::Columns::Cartn_x, StructFile::Columns::Cartn_y, StructFile::Columns::Cartn_z
    };

    using model_prefix_t = rc_simple_fixed<vector_io_stream, prefix_model_size, 1 << 14, 8>;
    using model_suffix_t = rc_simple<vector_io_stream, 1 << 12, 1>;
    using model_centroid_id_t = rc_simple<vector_io_stream, 1 << 10, 1>;
    using model_tetrahedron_first_t = rc_simple_fixed<vector_io_stream, 2, 1 << 12, 4>;

    using model_entry_type_t = rc_simple_fixed<vector_io_stream, 5, 1 << 12, 2>;
    using model_col_type_t = rc_simple_fixed<vector_io_stream, 6, 1 << 12, 4>;

    using model_aa_t = rc_simple_fixed<vector_io_stream, 22, 1 << 13, 1>;

    enum class entry_type_t { block = 0, loop_generic = 1, loop_atom = 2, loop_hetatm = 3, eod = 4 };

    // Cartesian conversion-relarted values
    int64_t max_error_backbone_uA_single_axis;
    int64_t max_error_sidechain_uA_single_axis;
    int64_t res_backbone_uA_single_axis = 0;
    int64_t res_sidechain_uA_single_axis = 0;

    ModelCompress cart_comp;
    ModelCompress cart_decomp;

    bool compression_mode;
    bool lossless;
    bool max_compression;
    bool single_bf;
    int cart_precision = 0;
    int64_t cart_working_multiplier = 0;

    vector<char> zstd_dict;
    vector<char> chain_dict;
    vector<char> zstd_dict_ext;

    enum class rc_int_class_t 
        {no_columns = 0, no_rows = 1, col_decimals = 2, col_width = 3, block_entry_size = 4, numeric_val = 5, numeric_diff = 6,
        aa_sequence_len = 7, bf_delta = 8, string_rep = 9};

    rc_context_vec_emb<model_prefix_t> dict_cart_prefix;
    rc_context_vec<model_suffix_t> dict_cart_suffix;
    rc_context_vec_emb<model_prefix_t> dict_main_prefix;
    rc_context_vec<model_suffix_t> dict_main_suffix;
    rc_context_vec<model_centroid_id_t> dict_centroid_id;
    rc_context_vec<model_tetrahedron_first_t> dict_tetrahedron_first;

    rc_context_vec<model_entry_type_t> dict_entry_type;
    rc_context_vec<model_col_type_t> dict_col_type;
    
    context_t ctx_entry_type;

    vector<uint8_t> v_rc;
    vector_io_stream *vios = nullptr;
    rc_encoder<vector_io_stream> *rce = nullptr;
    rc_decoder<vector_io_stream> *rcd = nullptr;

    model_prefix_t *tpl_prefix_enc = nullptr, * tpl_prefix_dec = nullptr;
    vector<model_suffix_t> tpl_suffix_enc, tpl_suffix_dec;
    model_col_type_t* tpl_col_type_enc = nullptr, *tpl_col_type_dec = nullptr;
    vector<model_centroid_id_t> tpl_centroid_id_enc, tpl_centroid_id_dec;
    model_tetrahedron_first_t *tpl_tetrahedron_first_enc = nullptr, *tpl_tetrahedron_first_dec = nullptr;

    ZSTD_CCtx* zstd_cctx;
    ZSTD_DCtx* zstd_dctx;
    ZSTD_CDict* zstd_cdict;
    ZSTD_DDict* zstd_ddict;

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

    void extend_zstd_dict(const string &struct_name);

    void compress_cart(vector<chain_desc_t>& chains);
    void decompress_cart(vector<chain_desc_t>& chains);

    void prepare_rc_tpl(bool compression);

    void add_aa_codes_to_dict(const vector<chain_desc_t>& chains);

    void rc_encode_int(uint32_t ctx, rc_context_vec_emb<model_prefix_t> &dict_prefix, rc_context_vec<model_suffix_t> &dict_suffix, int64_t val);
    int64_t rc_decode_int(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec<model_suffix_t>& dict_suffix);

    void dict_append(vector<char>& dict, const string& str, char sep)
    {
        dict.insert(dict.end(), str.begin(), str.end());
        dict.push_back(sep);
    }

    void dict_append(string &dict, const string& str)
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

    void determine_resolutions();

    void round_coordinates(atom_desc_t& atom_desc)
    {
        atom_desc.coords.x *= cart_working_multiplier;
        atom_desc.coords.y *= cart_working_multiplier;
        atom_desc.coords.z *= cart_working_multiplier;

        if (lossless)
            return;

        int64_t resolution = is_backbone_atom(atom_desc.type) ? res_backbone_uA_single_axis : res_sidechain_uA_single_axis;

        if (atom_desc.coords.x >= 0)
            atom_desc.coords.x = ((atom_desc.coords.x + resolution / 2) / resolution) * resolution;
        else
            atom_desc.coords.x = ((atom_desc.coords.x - resolution / 2) / resolution) * resolution;

        if (atom_desc.coords.y >= 0)
            atom_desc.coords.y = ((atom_desc.coords.y + resolution / 2) / resolution) * resolution;
        else
            atom_desc.coords.y = ((atom_desc.coords.y - resolution / 2) / resolution) * resolution;

        if (atom_desc.coords.z >= 0)
            atom_desc.coords.z = ((atom_desc.coords.z + resolution / 2) / resolution) * resolution;
        else
            atom_desc.coords.z = ((atom_desc.coords.z - resolution / 2) / resolution) * resolution;
    }

    col_type_t is_num_col_special(const vector<int>& vec);

    void enc_resolution(rc_encoder<vector_io_stream>* rce, int64_t val);
    int64_t dec_resolution(rc_decoder<vector_io_stream>* rcd);
    void enc_flag(rc_encoder<vector_io_stream>* rce, bool flag);
    bool dec_flag(rc_decoder<vector_io_stream>* rcd);

    template<typename T> void delete_var(T*& var)
    {
        if (var)
        {
            delete var;
            var = nullptr;
        }
    }

    void clear_rc_dict();

    void init_rc_types(bool compress);
    void release_rc_types();

    void compress_block_entry(const Entry* entry);
    void compress_loop_generic(const LoopEntry* le);
    void compress_loop_atom(const LoopEntry* le);
    void compress_loop_hetatm(const LoopEntry* le);
    void compress_string_column(model_col_type_t *mc, const StringColumn* col);
    void compress_numeric_column(model_col_type_t* mc, const NumericColumn* col);
    void compress_cart_column_as_numeric(model_col_type_t* mc, const NumericColumn* col);

    entry_type_t determine_loop_type(const StructFile* input, const LoopEntry* le);

    BlockEntry* decompress_block_entry(char *&str_data);
    LoopEntry *decompress_loop_generic(char*& str_data);
    LoopEntry* decompress_loop_atom(char*& str_data);
    LoopEntry* decompress_loop_hetatm(char*& str_data);
    AbstractColumn *decompress_string_column(const string &col_name, int no_rows, char*& str_data);
    AbstractColumn *decompress_string_column_special(const string &col_name, int no_rows, char*& str_data);
    AbstractColumn* decompress_numeric_column(col_type_t col_type, const string &col_name, int no_rows, vector<int> &v_tmp);
    AbstractColumn* decompress_cart_column_as_numeric(col_type_t col_type, const string &col_name, int no_rows, vector<int> &v_tmp);

    void append_cstr(char*& ptr, const char* src);

public:
    StructCompressor(bool compression_mode) :
        max_error_backbone_uA_single_axis(0),
        max_error_sidechain_uA_single_axis(0),
        compression_mode(compression_mode),
        lossless(true),
        max_compression(true),
        single_bf(false),
        zstd_cctx(nullptr),
        zstd_dctx(nullptr),
        zstd_cdict(nullptr),
        zstd_ddict(nullptr)
    {
        if (compression_mode)
        {
            zstd_cctx = ZSTD_createCCtx();
            ZSTD_CCtx_setParameter(zstd_cctx, ZSTD_c_compressionLevel, zstd_compression_level);
        }
        else
            zstd_dctx = ZSTD_createDCtx();

        init_rc_types(compression_mode);
    }

    ~StructCompressor()
    {
        if (zstd_cctx)
            ZSTD_freeCCtx(zstd_cctx);
        if (zstd_dctx)
            ZSTD_freeDCtx(zstd_dctx);
        if (zstd_cdict)
            ZSTD_freeCDict(zstd_cdict);
        if (zstd_ddict)
            ZSTD_freeDDict(zstd_ddict);

        release_rc_types();
    };

    // if 2nd parameter is absent, max_error is the same as for the backbone
    void set_max_error(int64_t _max_error_backbone_mA, int64_t _max_error_sidechain_mA = -1);
    void set_max_compression(bool _max_compression);
    void set_single_bf(bool _single_bf);

    void compress(const StructFile* input, vector<uint8_t>& packed, const string &struct_name);
    void decompress(StructFile* output, const vector<uint8_t>& packed, size_t raw_size, const string& struct_name);
};

// EOF
