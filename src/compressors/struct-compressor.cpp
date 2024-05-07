#include "../compressors/struct-compressor.h"

// *****************************************************************
StructCompressor::StructCompressor() :
    StructBase(),
    zstd_cctx(nullptr),
    zstd_cdict(nullptr)
{
    zstd_cctx = ZSTD_createCCtx();
    ZSTD_CCtx_setParameter(zstd_cctx, ZSTD_c_compressionLevel, zstd_compression_level);
    init_rc_types();
}

// *****************************************************************
StructCompressor::~StructCompressor()
{
    if (zstd_cctx)
        ZSTD_freeCCtx(zstd_cctx);
    if (zstd_cdict)
        ZSTD_freeCDict(zstd_cdict);
}


// *****************************************************************
void StructCompressor::set_max_error(int64_t max_error_backbone_mA, int64_t max_error_sidechain_mA)
{
    lossless = (max_error_backbone_mA == 0) && (max_error_sidechain_mA == 0);

    max_error_backbone_uA_single_axis = (int64_t)(1000.0 * max_error_backbone_mA / sqrt(3.0));

    if (max_error_sidechain_mA == -1)
        max_error_sidechain_uA_single_axis = max_error_backbone_uA_single_axis;
    else
        max_error_sidechain_uA_single_axis = (int64_t)(1000.0 * max_error_sidechain_mA / sqrt(3.0));
}

// *****************************************************************
void StructCompressor::set_max_compression(bool _max_compression)
{
    max_compression = _max_compression;
}

// *****************************************************************
void StructCompressor::set_single_bf(bool _single_bf)
{
    single_bf = _single_bf;
}

// *****************************************************************
void StructCompressor::determine_resolutions()
{
    cart_working_multiplier = pow10<int>(cart_working_precision - cart_precision);

    if (lossless)
    {
        res_backbone_uA_single_axis = cart_working_multiplier;
        res_sidechain_uA_single_axis = cart_working_multiplier;
    }
    else
    {
        res_backbone_uA_single_axis = 2 * max_error_backbone_uA_single_axis;
        res_sidechain_uA_single_axis = 2 * max_error_sidechain_uA_single_axis;
    }

    // Round to mA
    res_backbone_uA_single_axis = res_backbone_uA_single_axis / 1000 * 1000;
    res_sidechain_uA_single_axis = res_sidechain_uA_single_axis / 1000 * 1000;

    if (res_backbone_uA_single_axis < cart_working_multiplier)
        res_backbone_uA_single_axis = cart_working_multiplier;
    if (max_error_sidechain_uA_single_axis < cart_working_multiplier)
        max_error_sidechain_uA_single_axis = cart_working_multiplier;

    if (max_error_sidechain_uA_single_axis <= cart_working_multiplier && res_backbone_uA_single_axis < cart_working_multiplier)
        lossless = true;
}

// *****************************************************************
StructCompressor::col_type_t StructCompressor::is_num_col_special(const vector<int>& vec)
{
    if (vec.size() < 2)
        return col_type_t::numeric_general;

    int diff = vec[1] - vec[0];
    bool is_same_val = diff == 0;
    bool is_same_diff = true;

    for (size_t i = 2; i < vec.size() && (is_same_val || is_same_diff); ++i)
    {
        int cd = vec[i] - vec[i - 1];
        is_same_val &= (cd == 0);
        is_same_diff &= (cd == diff);
    }

    if (is_same_val)
        return col_type_t::numeric_same_values;
    if (is_same_diff)
        return col_type_t::numeric_same_diff;

    return col_type_t::numeric_general;
}

// *****************************************************************
void StructCompressor::rc_encode_uint(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec<model_suffix_t>& dict_suffix, int64_t val)
{
    auto nb = no_bits_max15(val);

    if (nb <= prefix_max_bits)
    {
        auto p_prefix = rc_find_context(dict_prefix, ctx, *tpl_prefix_enc);

        p_prefix->encode(nb);

        if (nb <= 1)
            return;

        val -= (((int64_t)1) << (nb - 1));

        if (nb < prefix_short_max_bits)
        {
            auto p_suffix = rc_find_context(dict_suffix, nb, &tpl_suffix_enc[nb - 1]);
            p_suffix->encode((uint32_t)val);
        }
        else
        {
            rce->encode_frequency(1, val, 1ull << (nb - 1));
        }
    }
    else
    {
        auto p_prefix = rc_find_context(dict_prefix, ctx, *tpl_prefix_enc);
        p_prefix->encode(prefix_max_bits + 1);

        val -= 1 << (prefix_max_bits - 1);

        int b1 = (int)(val & 0xff);
        int b2 = (int)((val >> 8) & 0xff);
        int b3 = (int)((val >> 16) & 0xff);
        int b4 = (int)(val >> 24);

        rc_find_context(dict_suffix, ctx * 16 + 13, &tpl_suffix_enc.back())->encode(b1);
        rc_find_context(dict_suffix, ctx * 16 + 14, &tpl_suffix_enc.back())->encode(b2);

        if(b4 == 0 && b3 < 255)
            rc_find_context(dict_suffix, ctx * 16 + 15, &tpl_suffix_enc.back())->encode(b3);
        else
        {
            rc_find_context(dict_suffix, ctx * 16 + 15, &tpl_suffix_enc.back())->encode(255);
            rc_find_context(dict_suffix, ctx * 16 + 15, &tpl_suffix_enc.back())->encode(b3);
            rc_find_context(dict_suffix, ctx * 16 + 15, &tpl_suffix_enc.back())->encode(b4);
        }
    }
}

// *****************************************************************
void StructCompressor::rc_encode_int(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec_emb<model_negative_t>& dict_negative, rc_context_vec<model_suffix_t>& dict_suffix, int64_t val)
{
    bool is_neg = false;

    if (val < 0)
    {
        is_neg = true;
        val = -val;
    }

    rc_encode_uint(ctx, dict_prefix, dict_suffix, val);

    if (val != 0)
    {
        //        auto p_negative = rc_find_context(dict_negative, ctx, *tpl_negative_enc);
        //        p_negative->encode((int)is_neg);
        rce->encode_frequency(1, (int)is_neg, 2);
    }
}

// *****************************************************************
void StructCompressor::enc_resolution(rc_encoder<vector_io_stream>* rce, int64_t val)
{
    /*    uint64_t x1 = val & 0xff;
        val >>= 8;
        uint64_t x2 = val & 0xff;
        val >>= 8;
        uint64_t x3 = val & 0xff;

        rce->encode_frequency(1, x1, 256);
        rce->encode_frequency(1, x2, 256);
        rce->encode_frequency(1, x3, 256);*/

    val /= 1000;

    //    rce->encode_frequency(1, val & 0xffu, 1001);
    rce->encode_frequency(1, val, 1001);
}

// *****************************************************************
void StructCompressor::enc_flag(rc_encoder<vector_io_stream>* rce, bool flag)
{
    rce->encode_frequency(1, (int)flag, 2);
}

// *****************************************************************
void StructCompressor::compress(const StructFileInput* input, vector<uint8_t>& packed, const string& struct_name)
{
    clear_rc_dict();
    clear_serializers();

    rce->start();

    model_entry_type_t tpl_entry_type(rce, nullptr, true);

    prepare_rc_tpl();

    ctx_entry_type = 0;

    loop_atom_sections.clear();
    aa_sequences.clear();

    // Determine AA sequences
    for (const Entry* e : input->getEntries())
    {
        if (e->isLoop)
        {
            const LoopEntry* le = dynamic_cast<const LoopEntry*>(e);

            entry_type_t entry_type = determine_loop_type(input, le);

            if (entry_type == entry_type_t::loop_atom)
            {
                for (const auto& chain : protein.get_chains())
                    for (const auto& aa : chain.aa)
                        aa_sequences.emplace_back(aa.type);

                loop_atom_sections[le] = move(protein);
            }
        }
    }

    // Encode AA sequences
    model_aa_t ctx_aa(rce, nullptr, true);

    rc_encode_uint((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix, aa_sequences.size());
    for (const auto x : aa_sequences)
        ctx_aa.encode((uint8_t)x);

    // Do the compression
    for (const Entry* e : input->getEntries())
    {
        auto p_entry_type = rc_find_context(dict_entry_type, ctx_entry_type, &tpl_entry_type);

        if (!e->isLoop)
        {
            p_entry_type->encode((uint8_t)entry_type_t::block);
            ctx_entry_type = ((ctx_entry_type << 3) + (uint8_t)entry_type_t::block) & 0x7u;

            compress_block_entry(e);
        }
        else if (e->isLoop)
        {
            const LoopEntry* le = dynamic_cast<const LoopEntry*>(e);

            entry_type_t entry_type = determine_loop_type(input, le);

            if (entry_type == entry_type_t::loop_atom)
            {
                p_entry_type->encode((uint8_t)entry_type_t::loop_atom);
                ctx_entry_type = ((ctx_entry_type << 3) + (uint8_t)entry_type_t::loop_atom) & 0x7u;

                compress_loop_atom(le);
            }
            /*            else if ((c->name == input->getHetatmEntryName()))
                        {
                            p_entry_type->encode((uint8_t) entry_type_t::loop_hetatm);
                            ctx_entry_type = ((ctx_entry_type << 3) + (uint8_t)entry_type_t::hetatm) & 0x7u;

                            compress_loop_hetatm(le);
                        }*/
            else
            {
                p_entry_type->encode((uint8_t)entry_type_t::loop_generic);
                ctx_entry_type = ((ctx_entry_type << 3) + (uint8_t)entry_type_t::loop_generic) & 0x7u;

                compress_loop_generic(le);
            }
        }
    }

    auto p_entry_type = rc_find_context(dict_entry_type, ctx_entry_type, &tpl_entry_type);
    p_entry_type->encode((uint8_t)entry_type_t::eod);

    rce->complete();
    ser_rc.append32u((uint32_t)v_rc.size());
    ser_rc.append_bytes((char*)v_rc.data(), v_rc.size());

    ser_plain.append_serializer(ser_strings);
    ser_plain.append_serializer(ser_text);

    extend_zstd_dict(struct_name);

    ser_plain.compress_zstd(zstd_cctx, &zstd_dict_ext);

    ser_full.append_serializer(ser_plain);
    ser_full.append_serializer(ser_rc);

    packed.swap(ser_full.get_data());

    clear_serializers();
}

// *****************************************************************
StructCompressor::entry_type_t StructCompressor::determine_loop_type(const StructFileInput* input, const LoopEntry* le)
{
    if (le->getType() == LoopEntry::Type::Atom)
    {
        auto p = loop_atom_sections.find(le);

        if (p != loop_atom_sections.end())
        {
            protein = move(p->second);
            loop_atom_sections.erase(p);

            return entry_type_t::loop_atom;
        }

        if (!protein.parse_chains(le))
            return entry_type_t::loop_generic;

        if (!protein.all_chains_valid_seq_id())
            return entry_type_t::loop_generic;

        typicality_t typical = typicality_t::all_mask;

        for (const auto& chain : protein.get_chains())
            typical &= chain.typical;

        //        if (typical.check_all())
        if (typical.check_basic())
            return entry_type_t::loop_atom;

        return entry_type_t::loop_generic;

    }
    /*    else if (le->type == LoopEntry::Type::Hetatm)
        {

        }*/

    return entry_type_t::loop_generic;
}

// *****************************************************************
void StructCompressor::compress_block_entry(const Entry* entry)
{
    const BlockEntry* be = dynamic_cast<const BlockEntry*>(entry);

    rc_encode_uint((uint32_t)rc_int_class_t::block_entry_size, dict_main_prefix, dict_main_suffix, be->size);

    ser_text.append_bytes(be->data, be->size);
    ser_text.append8u(0);
}

// *****************************************************************
void StructCompressor::compress_string_column(model_col_type_t* mc, const StringColumn* col)
{
    mc->encode((uint8_t)col_type_t::text);          // string col
    ser_strings.append_str(col->name);
    rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix, col->width);

    vector<int> no_reps;

    char* prev_start, * prev_end, * col_end;
    char* curr_start, * curr_end;

    vector<uint32_t> v_rep;

    col_end = col->raw + col->size_bytes;
    prev_start = col->raw;

    prev_end = find(prev_start, col_end, 0);
    //    curr_start = prev_start + col->width + 1;
    curr_start = prev_end + 1;

    ser_text.append_bytes(prev_start, prev_end - prev_start + 1);
    v_rep.emplace_back(0);

    while (curr_start < col_end)
    {
        curr_end = find(curr_start, col_end, 0);

        if (curr_end - curr_start == prev_end - prev_start && equal(prev_start, prev_end, curr_start))
            ++v_rep.back();
        else
        {
            ser_text.append_bytes(curr_start, curr_end - curr_start + 1);
            v_rep.emplace_back(0);
        }

        prev_start = curr_start;
        prev_end = curr_end;
        //        curr_start += col->width + 1;
        curr_start = prev_end + 1;
    }

    // !!! TODO: improve this coding
    for (const auto rep : v_rep)
        rc_encode_uint((uint32_t)rc_int_class_t::string_rep, dict_main_prefix, dict_main_suffix, rep);
}

// *****************************************************************
void StructCompressor::compress_numeric_column(model_col_type_t* mc, const NumericColumn* col)
{
    auto vals = col->getValues();
    auto col_type = is_num_col_special(vals);

    mc->encode((uint8_t)col_type);

    ser_strings.append_str(col->name);

    rc_encode_uint((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix, col->numDecimals);
    rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix, col->width);

    if (col_type == col_type_t::numeric_same_values)
    {
        rc_encode_int((uint32_t)rc_int_class_t::numeric_val, dict_main_prefix, dict_main_negative, dict_main_suffix, vals[0]);
    }
    else if (col_type == col_type_t::numeric_same_diff)
    {
        rc_encode_int((uint32_t)rc_int_class_t::numeric_val, dict_main_prefix, dict_main_negative, dict_main_suffix, vals[0]);
        rc_encode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_negative, dict_main_suffix, vals[1] - vals[0]);
    }
    else
    {
        int p_val = 0;
        for (auto x : vals)
        {
            rc_encode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_negative, dict_main_suffix, x - p_val);
            p_val = x;
        }
    }
}

// get --type ALL --all -t 1 --in aa.psa  --outdir tmp/ -v 1 
// add --type pdb -t 1 --infile MGYP001505626013.pdb.gz --out aa.psa --lossy --max-error-bb 10 --max-error-sc 100
// 
// *****************************************************************
void StructCompressor::compress_cart_column_as_numeric(model_col_type_t* mc, const NumericColumn* col)
{
    mc->encode((uint8_t)col_type_t::cart_as_numeric);

    ser_strings.append_str(col->name);

    rc_encode_uint((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix, col->numDecimals);
    rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix, col->width);

    auto vals = col->getValues();

    cart_precision = dynamic_cast<const NumericColumn*>(col)->numDecimals;
    rce->encode_frequency(1, cart_precision, 7);

    determine_resolutions();

    enc_resolution(rce, res_backbone_uA_single_axis);

    int64_t resolution = res_backbone_uA_single_axis;

    for (auto& x : vals)
    {
        int64_t x64 = x;

        x64 *= (int) cart_working_multiplier;

        if (x64 >= 0)
            x64 = ((x64 + resolution / 2) / resolution);
        else
            x64 = ((x64 - resolution / 2) / resolution);

        x = (int)x64;
    }

    int p_val = 0;
    for (auto x : vals)
    {
        rc_encode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_negative, dict_main_suffix, x - p_val);
        p_val = x;
    }
}

// *****************************************************************
void StructCompressor::compress_loop_generic(const LoopEntry* le)
{
    ser_strings.append_str(le->name);

    rc_encode_uint((uint32_t)rc_int_class_t::no_columns, dict_main_prefix, dict_main_suffix, le->getColumns().size());
    rc_encode_uint((uint32_t)rc_int_class_t::no_rows, dict_main_prefix, dict_main_suffix, le->getRowCount());

    auto p_col_type = rc_find_context(dict_col_type, 0, tpl_col_type_enc);

    for (const AbstractColumn* c : le->getColumns())
        if (c->isNumeric)
        {
            if (!lossless && (c->name == StructFileBase::Columns::Cartn_x || c->name == StructFileBase::Columns::Cartn_y || c->name == StructFileBase::Columns::Cartn_z))
                compress_cart_column_as_numeric(p_col_type, dynamic_cast<const NumericColumn*>(c));
            else
                compress_numeric_column(p_col_type, dynamic_cast<const NumericColumn*>(c));
        }
        else
            compress_string_column(p_col_type, dynamic_cast<const StringColumn*>(c));
}

// *****************************************************************
void StructCompressor::compress_loop_atom(const LoopEntry* le)
{
    ser_strings.append_str(le->name);

    rc_encode_uint((uint32_t)rc_int_class_t::no_columns, dict_main_prefix, dict_main_suffix, le->getColumns().size());
    rc_encode_uint((uint32_t)rc_int_class_t::no_rows, dict_main_prefix, dict_main_suffix, le->getRowCount());

    auto& chains = protein.get_chains();

    for (auto& chain : chains)
    {
        int bf_prev = 0;
        typicality_t typicality = chain.typical;

        rc_encode_uint((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix, chain.aa.size());
        rce->encode_frequency(1, chain.id, 128);

        bool is_constant_bf = typicality.check_B_factor();
        bool is_oxt = typicality.check_oxt();

        if (!is_constant_bf && single_bf)
        {
            // Calculate avg of B-factors for each AA
            for (const auto& aa : chain.aa)
            {
                int bf = 0;
                for (const auto& atom : aa.atoms)
                    bf += atom.B_factor;
                bf += (int) aa.atoms.size() / 2;
                bf /= (int) aa.atoms.size();
            }

            is_constant_bf = true;
        }

        rce->encode_frequency(1, (int)is_constant_bf, 2);
        rce->encode_frequency(1, (int)is_oxt, 2);

        if (is_constant_bf)
        {
            for (const auto& aa : chain.aa)
            {
                int bf_curr = aa.atoms.front().B_factor;
                int bf_delta = bf_curr - bf_prev;
                bf_prev = bf_curr;

                rc_encode_int((uint32_t)rc_int_class_t::bf_delta, dict_main_prefix, dict_main_negative, dict_main_suffix, bf_delta);
            }
        }
        else
        {
            for (const auto& aa : chain.aa)
                for (const auto& atom : aa.atoms)
                {
                    int bf_curr = atom.B_factor;
                    int bf_delta = bf_curr - bf_prev;
                    bf_prev = bf_curr;

                    rc_encode_int((uint32_t)rc_int_class_t::bf_delta, dict_main_prefix, dict_main_negative, dict_main_suffix, bf_delta);
                }
        }
    }

    rc_encode_uint((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix, 0);

    cart_precision = dynamic_cast<const NumericColumn*>(le->findColumn(StructFileBase::Columns::Cartn_x))->numDecimals;
    rce->encode_frequency(1, cart_precision, 7);

    determine_resolutions();
    compress_cart(chains);

    auto p_col_type = rc_find_context(dict_col_type, 0, tpl_col_type_enc);

    for (const AbstractColumn* c : le->getColumns())
    {
        const auto p = chain_desc_col.find(c->name);

        if (p != chain_desc_col.end())
        {
            rce->encode_frequency(1, distance(chain_desc_col.begin(), p), chain_desc_col.size() + 1);

            if (chain_desc_col_numeric.count(c->name))
            {
                const NumericColumn* col = dynamic_cast<const NumericColumn*>(c);

                rc_encode_uint((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix, col->numDecimals);
                rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix, col->width);
            }
            else
            {
                const StringColumn* col = dynamic_cast<const StringColumn*>(c);

                rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix, col->width);
            }
        }
        else
        {
            rce->encode_frequency(1, chain_desc_col.size(), chain_desc_col.size() + 1);

            if (c->isNumeric)
                compress_numeric_column(p_col_type, dynamic_cast<const NumericColumn*>(c));
            else
                compress_string_column(p_col_type, dynamic_cast<const StringColumn*>(c));
        }
    }
}

// *****************************************************************
void StructCompressor::compress_loop_hetatm(const LoopEntry* le)
{

}

// *****************************************************************
void StructCompressor::compress_cart(vector<chain_desc_t>& chains)
{
#ifdef NO_CART_STORAGE
    return;
#endif

    int centroid_id;
    int no_centroids;
    bool first;
    bool equal_q;
    int64_t dx;
    int64_t dy;
    int64_t dz;

    model_ok_t rce_ok(rce, model_ok_init, true);

    enc_flag(rce, lossless);
    if (!lossless)
    {
        enc_resolution(rce, res_backbone_uA_single_axis);
        enc_resolution(rce, res_sidechain_uA_single_axis);
    }
    else
    {
        res_backbone_uA_single_axis = cart_working_multiplier;
        res_sidechain_uA_single_axis = cart_working_multiplier;
    }

    for (auto& chain : chains)
        for (auto& aa : chain.aa)
            for (size_t i = 0; i < aa.atoms.size(); ++i)
                round_coordinates(aa.atoms[i]);

    cart_comp.set_resolution(res_backbone_uA_single_axis, res_sidechain_uA_single_axis);
    cart_comp.set_max_compression(max_compression);

    if (cart_comp.clean_model())
        remake_init_tpls();

    for (auto& chain : chains)
    {
        uint32_t n_in_chain = 0;
        int_coords_t prev_atom_coords{ 0, 0, 0 };

        cart_comp.restart_chain();

        for (auto& aa : chain.aa)
        {
            cart_comp.new_aa(aa);

            for (size_t i = 0; i < aa.atoms.size(); ++i)
            {
                bool ok = cart_comp.predict((int)i, centroid_id, no_centroids, first, equal_q, dx, dy, dz);

                if (n_in_chain++ > 2)
                    rce_ok.encode((int)ok);

                uint32_t ctx = cart_comp.calc_ctx(aa.type, aa.atoms[i].type);

                if (ok)
                {
                    if (no_centroids > 1)
                    {
                        //                        auto p_centroid_id = rc_find_context(dict_centroid_id, no_centroids, &tpl_centroid_id_enc[no_centroids]);

                        auto x = cart_comp.packed_atom_ctx(aa.type, aa.atoms[i].type);
                        auto p_centroid_id = rc_find_context(dict_centroid_id, x, &tpl_centroid_id_init_enc[x]);

                        p_centroid_id->encode(centroid_id);
                    }

                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix, dx);
                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix, dy);
                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix, dz);

                    if (!equal_q)
                    {
                        auto p_first = rc_find_context(dict_tetrahedron_first, cart_comp.packed_atom_ctx(aa.type, aa.atoms[i].type), tpl_tetrahedron_first_enc);
                        p_first->encode((int)first);
                    }
                }
                else
                {
                    int64_t resolution = is_backbone_atom(aa.atoms[i].type) ? res_backbone_uA_single_axis : res_sidechain_uA_single_axis;

                    /*                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix, aa.atoms[i].coords.x / resolution);
                                        rc_encode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix, aa.atoms[i].coords.y / resolution);
                                        rc_encode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix, aa.atoms[i].coords.z / resolution);*/

                    dx = aa.atoms[i].coords.x - prev_atom_coords.x;
                    dy = aa.atoms[i].coords.y - prev_atom_coords.y;
                    dz = aa.atoms[i].coords.z - prev_atom_coords.z;

                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix, dx / resolution);
                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix, dy / resolution);
                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix, dz / resolution);
                }

                prev_atom_coords = aa.atoms[i].coords;
            }
        }
    }
}

// *****************************************************************
void StructCompressor::init_rc_types()
{
    vios = new vector_io_stream(v_rc);

    rce = new rc_encoder<vector_io_stream>(*vios);
}

// *****************************************************************
void StructCompressor::release_rc_types()
{
    delete_var(vios);
    delete_var(rce);

    delete_var(tpl_prefix_enc);
    delete_var(tpl_negative_enc);
    delete_var(tpl_col_type_enc);
    delete_var(tpl_tetrahedron_first_enc);
    tpl_suffix_enc.clear();
    tpl_centroid_id_enc.clear();
    tpl_centroid_id_init_enc.clear();
}

// *****************************************************************
void StructCompressor::prepare_rc_tpl()
{
    if (!tpl_col_type_enc)
        tpl_col_type_enc = new model_col_type_t(rce, nullptr, true);
    if (!tpl_prefix_enc)
        tpl_prefix_enc = new model_prefix_t(rce, nullptr, true);
    if (!tpl_negative_enc)
        tpl_negative_enc = new model_negative_t(rce, nullptr, true);
    if (!tpl_tetrahedron_first_enc)
        tpl_tetrahedron_first_enc = new model_tetrahedron_first_t(rce, nullptr, true);

    if (tpl_suffix_enc.empty())
    {
        for (uint32_t i = 0; i <= prefix_short_max_bits; ++i)
            tpl_suffix_enc.push_back(model_suffix_t(rce, 1 << i, nullptr, true));
        tpl_suffix_enc.push_back(model_suffix_t(rce, 256, nullptr, true));
    }

    if (tpl_centroid_id_enc.empty())
        for (int i = 0; i <= cart_comp.max_no_centroids; ++i)
            tpl_centroid_id_enc.push_back(model_centroid_id_t(rce, i, nullptr, true));

    if (tpl_centroid_id_init_enc.empty())
        for (int i = 0; i <= (int) cart_comp.max_packed_atom_ctx; ++i)
        {
            auto cc = cart_comp.get_centroid_counts(i);

            if (cc.first)
                tpl_centroid_id_init_enc.push_back(model_centroid_id_t(rce, (uint32_t) cc.second, cc.first, true));
            else
                tpl_centroid_id_init_enc.push_back(model_centroid_id_t(rce, 2, nullptr, true));     /// will not be used
        }
}

// *****************************************************************
void StructCompressor::remake_init_tpls()
{
    tpl_centroid_id_init_enc.clear();

    for (int i = 0; i <= (int) cart_comp.max_packed_atom_ctx; ++i)
    {
        auto cc = cart_comp.get_centroid_counts(i);

        if (cc.first)
            tpl_centroid_id_init_enc.push_back(model_centroid_id_t(rce, (uint32_t) cc.second, cc.first, true));
        else
            tpl_centroid_id_init_enc.push_back(model_centroid_id_t(rce, 2, nullptr, true));     // will not be used
    }
}


