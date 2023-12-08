#pragma once

#include "../compressors/struct-base.h"
#include "../parsers/input-load.h"

class StructCompressor : public StructBase
{
protected:
    bool compression_mode;

    ZSTD_CCtx* zstd_cctx;
    ZSTD_CDict* zstd_cdict;

    ModelCompress cart_comp;

    model_prefix_t* tpl_prefix_enc = nullptr;
    model_negative_t* tpl_negative_enc = nullptr;
    vector<model_suffix_t> tpl_suffix_enc;
    model_col_type_t* tpl_col_type_enc = nullptr;
    vector<model_centroid_id_t> tpl_centroid_id_enc;
    vector<model_centroid_id_t> tpl_centroid_id_init_enc;
    model_tetrahedron_first_t* tpl_tetrahedron_first_enc = nullptr;

    rc_encoder<vector_io_stream>* rce = nullptr;

    void prepare_rc_tpl();
    void remake_init_tpls();

    void init_rc_types();
    void release_rc_types();

    void compress_block_entry(const Entry* entry);
    void compress_loop_generic(const LoopEntry* le);
    void compress_loop_atom(const LoopEntry* le);
    void compress_loop_hetatm(const LoopEntry* le);
    void compress_string_column(model_col_type_t* mc, const StringColumn* col);
    void compress_numeric_column(model_col_type_t* mc, const NumericColumn* col);
    void compress_cart_column_as_numeric(model_col_type_t* mc, const NumericColumn* col);

    entry_type_t determine_loop_type(const StructFileInput* input, const LoopEntry* le);

    col_type_t is_num_col_special(const vector<int>& vec);

    void rc_encode_int(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec_emb<model_negative_t>& dict_negative, rc_context_vec<model_suffix_t>& dict_suffix, int64_t val);
    void rc_encode_uint(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec<model_suffix_t>& dict_suffix, int64_t val);

    void enc_resolution(rc_encoder<vector_io_stream>* rce, int64_t val);
    void enc_flag(rc_encoder<vector_io_stream>* rce, bool flag);

    void compress_cart(vector<chain_desc_t>& chains);

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

public:
    StructCompressor();
    ~StructCompressor();

    // if 2nd parameter is absent, max_error is the same as for the backbone
    void set_max_error(int64_t _max_error_backbone_mA, int64_t _max_error_sidechain_mA = -1);
    void set_max_compression(bool _max_compression);
    void set_single_bf(bool _single_bf);

    void compress(const StructFileInput* input, vector<uint8_t>& packed, const string& struct_name);

};