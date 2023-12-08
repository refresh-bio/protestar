#pragma once
#include "../compressors/struct-base.h"
#include "../parsers/input-save.h"

class StructDecompressor : public StructBase
{
protected:
    bool compression_mode;

    ZSTD_DCtx* zstd_dctx;
    ZSTD_DDict* zstd_ddict;

    ModelCompress cart_decomp;

    model_prefix_t* tpl_prefix_dec = nullptr;
    model_negative_t* tpl_negative_dec = nullptr;
    vector<model_suffix_t> tpl_suffix_dec;
    model_col_type_t* tpl_col_type_dec = nullptr;
    vector<model_centroid_id_t> tpl_centroid_id_dec;
    vector<model_centroid_id_t> tpl_centroid_id_init_dec;
    model_tetrahedron_first_t* tpl_tetrahedron_first_dec = nullptr;

    rc_decoder<vector_io_stream>* rcd = nullptr;

    void prepare_rc_tpl();
    void remake_init_tpls();

    void init_rc_types();
    void release_rc_types();

    BlockEntry* decompress_block_entry(char*& str_data);
    LoopEntry* decompress_loop_generic(char*& str_data);
    LoopEntry* decompress_loop_atom(char*& str_data);
    LoopEntry* decompress_loop_hetatm(char*& str_data);
    AbstractColumn* decompress_string_column(const string& col_name, int no_rows, char*& str_data);
    AbstractColumn* decompress_string_column_special(const string& col_name, int no_rows, char*& str_data);
    AbstractColumn* decompress_numeric_column(col_type_t col_type, const string& col_name, int no_rows, vector<int>& v_tmp);
    AbstractColumn* decompress_cart_column_as_numeric(col_type_t col_type, const string& col_name, int no_rows, vector<int>& v_tmp);

    int64_t rc_decode_int(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec_emb<model_negative_t>& dict_negative, rc_context_vec<model_suffix_t>& dict_suffix);
    int64_t rc_decode_uint(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec<model_suffix_t>& dict_suffix);

    int64_t dec_resolution(rc_decoder<vector_io_stream>* rcd);
    bool dec_flag(rc_decoder<vector_io_stream>* rcd);

    void decompress_cart(vector<chain_desc_t>& chains);

    void append_cstr(char*& ptr, const char* src);

public:
    StructDecompressor() :
        StructBase(),
        zstd_dctx(nullptr),
        zstd_ddict(nullptr)
    {
        zstd_dctx = ZSTD_createDCtx();
        init_rc_types();
    }

    ~StructDecompressor()
    {
        if (zstd_dctx)
            ZSTD_freeDCtx(zstd_dctx);
        if (zstd_ddict)
            ZSTD_freeDDict(zstd_ddict);
    }

    void decompress(StructFileOutput* output, const vector<uint8_t>& packed, size_t raw_size, const string& struct_name);

};