#include "struct-compressor.h"

#include <iterator>

// *****************************************************************
void StructCompressor::set_max_error(int64_t max_error_backbone_mA, int64_t max_error_sidechain_mA)
{
    lossless = (max_error_backbone_mA == 0) && (max_error_sidechain_mA == 0);

    max_error_backbone_uA_single_axis = (int64_t) (1000.0 * max_error_backbone_mA / sqrt(3.0));

    if (max_error_sidechain_mA == -1)
        max_error_sidechain_uA_single_axis = max_error_backbone_uA_single_axis;
    else
        max_error_sidechain_uA_single_axis = (int64_t) (1000.0 * max_error_sidechain_mA / sqrt(3.0));
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
void StructCompressor::rc_encode_int(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec<model_suffix_t>& dict_suffix, int64_t val)
{
    bool is_neg = false;

    if (val < 0)
    {
        is_neg = true;
        val = -val;
    }

    auto nb = no_bits_max15(val);

    if (nb < prefix_max_bits)
    {
        auto p_prefix = rc_find_context(dict_prefix, ctx, *tpl_prefix_enc);

        p_prefix->encode(nb * 2 - (int)is_neg);

        if (nb <= 1)
            return;

        val -= (((int64_t)1) << (nb - 1));

        if (nb < prefix_short_max_bits)
        {
            auto p_suffix = rc_find_context(dict_suffix, ctx * 16 + nb, &tpl_suffix_enc[nb - 1]);
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
        p_prefix->encode(2 * prefix_max_bits + 1 + (int)is_neg);

        val -= 1 << (prefix_max_bits - 1);

        int b1 = (int) (val & 0xff);
        int b2 = (int) ((val >> 8) & 0xff);
        int b3 = (int) (val >> 16);

        rc_find_context(dict_suffix, ctx * 16 + 13, &tpl_suffix_enc.back())->encode(b1);
        rc_find_context(dict_suffix, ctx * 16 + 14, &tpl_suffix_enc.back())->encode(b2);
        rc_find_context(dict_suffix, ctx * 16 + 15, &tpl_suffix_enc.back())->encode(b3);
    }
}

// *****************************************************************
int64_t StructCompressor::rc_decode_int(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec<model_suffix_t>& dict_suffix)
{
    bool is_neg = false;
    int val = 0;

    auto p_prefix = rc_find_context(dict_prefix, ctx, *tpl_prefix_dec);
    auto nb = p_prefix->decode();

    if (nb < 2 * prefix_max_bits - 1)
    {
        if (nb & 1)
        {
            is_neg = true;
            nb = (nb + 1) / 2;
        }
        else
            nb = nb / 2;

        if (nb == 0)
            return 0;
        else if (nb == 1)
            return is_neg ? -1 : 1;

        if (nb < prefix_short_max_bits)
        {
            auto p_suffix = rc_find_context(dict_suffix, ctx * 16 + nb, &tpl_suffix_dec[nb - 1]);
            val = p_suffix->decode();
        }
        else
        {
            val = (int) rcd->get_cumulative_freq(1ull << (nb - 1));
            rcd->update_frequency(1, val, 1ull << (nb - 1));
        }

        val += (1 << (nb - 1));
    }
    else
    {
        uint32_t b1 = rc_find_context(dict_suffix, ctx * 16 + 13, &tpl_suffix_dec.back())->decode();
        uint32_t b2 = rc_find_context(dict_suffix, ctx * 16 + 14, &tpl_suffix_dec.back())->decode();
        uint32_t b3 = rc_find_context(dict_suffix, ctx * 16 + 15, &tpl_suffix_dec.back())->decode();

        val = b3 * 65536 + b2 * 256 + b1;

        val += 1 << (prefix_max_bits - 1);

        is_neg = (nb == 2 * prefix_max_bits + 1 + 1);
    }

    return is_neg ? -val : val;
}

// *****************************************************************
void StructCompressor::enc_resolution(rc_encoder<vector_io_stream>* rce, int64_t val)
{
    val /= 1000;

    rce->encode_frequency(1, val, 1001);
}

// *****************************************************************
int64_t StructCompressor::dec_resolution(rc_decoder<vector_io_stream>* rcd)
{
    uint64_t r = rcd->get_cumulative_freq(1001);
    rcd->update_frequency(1, r, 1001);

    return r * 1000;
}

// *****************************************************************
void StructCompressor::enc_flag(rc_encoder<vector_io_stream>* rce, bool flag)
{
    rce->encode_frequency(1, (int)flag, 2);
}

// *****************************************************************
bool StructCompressor::dec_flag(rc_decoder<vector_io_stream>* rcd)
{
    bool flag = rcd->get_cumulative_freq(2);
    rcd->update_frequency(1, (int)flag, 2);

    return flag;
}

// *****************************************************************
void StructCompressor::clear_rc_dict()
{
    dict_cart_prefix.clear();
    dict_cart_suffix.clear();
    dict_main_prefix.clear();
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
void StructCompressor::add_aa_codes_to_dict(const vector<chain_desc_t>& chains)
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
void StructCompressor::compress(const StructFile* input, vector<uint8_t>& packed, const string& struct_name)
{
    clear_rc_dict();
    clear_serializers();

    rce->start();

    model_entry_type_t tpl_entry_type(rce, nullptr, true);

    prepare_rc_tpl(true);

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
                for (const auto &chain : protein.get_chains())
                    for (const auto &aa : chain.aa)
                        aa_sequences.emplace_back(aa.type);

                loop_atom_sections[le] = move(protein);
            }
        }
    }

    // Encode AA sequences
    model_aa_t ctx_aa(rce, nullptr, true);

    rc_encode_int((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix, aa_sequences.size());
    for(const auto x : aa_sequences)
        ctx_aa.encode((uint8_t)x);

    // Do the compression
    for (const Entry* e : input->getEntries())
    {
        auto p_entry_type = rc_find_context(dict_entry_type, ctx_entry_type, &tpl_entry_type);

        if (!e->isLoop)
        {
            p_entry_type->encode((uint8_t) entry_type_t::block);
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
StructCompressor::entry_type_t StructCompressor::determine_loop_type(const StructFile* input, const LoopEntry* le)
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

        if(!protein.all_chains_valid_seq_id())
            return entry_type_t::loop_generic;

        typicality_t typical = typicality_t::all_mask;

        for (const auto &chain : protein.get_chains())
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

    rc_encode_int((uint32_t)rc_int_class_t::block_entry_size, dict_main_prefix, dict_main_suffix, be->size);

    ser_text.append_bytes(be->data, be->size);
    ser_text.append8u(0);
}

// *****************************************************************
void StructCompressor::compress_string_column(model_col_type_t* mc, const StringColumn* col)
{
    mc->encode((uint8_t)col_type_t::text);          // string col
    ser_strings.append_str(col->name);
    rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix, col->width);

    vector<int> no_reps;

    char *prev_start, *prev_end, *col_end;
    char *curr_start, *curr_end;

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
        rc_encode_int((uint32_t)rc_int_class_t::string_rep, dict_main_prefix, dict_main_suffix, rep);
}

// *****************************************************************
void StructCompressor::compress_numeric_column(model_col_type_t* mc, const NumericColumn* col)
{
    auto vals = col->getValues();
    auto col_type = is_num_col_special(vals);

    mc->encode((uint8_t)col_type);          

    ser_strings.append_str(col->name);

    rc_encode_int((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix, col->numDecimals);
    rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix, col->width);

    if (col_type == col_type_t::numeric_same_values)
    {
        rc_encode_int((uint32_t)rc_int_class_t::numeric_val, dict_main_prefix, dict_main_suffix, vals[0]);
    }
    else if (col_type == col_type_t::numeric_same_diff)
    {
        rc_encode_int((uint32_t)rc_int_class_t::numeric_val, dict_main_prefix, dict_main_suffix, vals[0]);
        rc_encode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_suffix, vals[1] - vals[0]);
    }
    else
    {
        int p_val = 0;
        for (auto x : vals)
        {
            rc_encode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_suffix, x - p_val);
            p_val = x;
        }
    }
}

// *****************************************************************
void StructCompressor::compress_cart_column_as_numeric(model_col_type_t* mc, const NumericColumn* col)
{
    mc->encode((uint8_t)col_type_t::cart_as_numeric);

    ser_strings.append_str(col->name);

    rc_encode_int((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix, col->numDecimals);
    rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix, col->width);

    auto vals = col->getValues();

    cart_precision = dynamic_cast<const NumericColumn*>(col)->numDecimals;
    rce->encode_frequency(1, cart_precision, 7);

    determine_resolutions();

    enc_resolution(rce, res_backbone_uA_single_axis);

    int64_t resolution = res_backbone_uA_single_axis;

    for (auto& x : vals)
    {
        x *= cart_working_multiplier;

        if (x >= 0)
            x = ((x + resolution / 2) / resolution);
        else
            x = ((x - resolution / 2) / resolution);
    }

    int p_val = 0;
    for (auto x : vals)
    {
        rc_encode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_suffix, x - p_val);
        p_val = x;
    }
}

// *****************************************************************
void StructCompressor::compress_loop_generic(const LoopEntry* le)
{
    ser_strings.append_str(le->name);

    rc_encode_int((uint32_t) rc_int_class_t::no_columns, dict_main_prefix, dict_main_suffix, le->getColumns().size());
    rc_encode_int((uint32_t)rc_int_class_t::no_rows, dict_main_prefix, dict_main_suffix, le->getRowCount());

    auto p_col_type = rc_find_context(dict_col_type, 0, tpl_col_type_enc);

    for (const AbstractColumn* c : le->getColumns()) 
        if (c->isNumeric)
        {
            if (!lossless && (c->name == StructFile::Columns::Cartn_x || c->name == StructFile::Columns::Cartn_y || c->name == StructFile::Columns::Cartn_z))
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

    rc_encode_int((uint32_t)rc_int_class_t::no_columns, dict_main_prefix, dict_main_suffix, le->getColumns().size());
    rc_encode_int((uint32_t)rc_int_class_t::no_rows, dict_main_prefix, dict_main_suffix, le->getRowCount());

    auto& chains = protein.get_chains();

    for (auto& chain : chains)
    {
        int bf_prev = 0;
        typicality_t typicality = chain.typical;

        rc_encode_int((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix, chain.aa.size());
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
                bf += aa.atoms.size() / 2;
                bf /= aa.atoms.size();
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

                rc_encode_int((uint32_t)rc_int_class_t::bf_delta, dict_main_prefix, dict_main_suffix, bf_delta);
            }
        }
        else
        {
            for (const auto& aa : chain.aa)
                for(const auto &atom : aa.atoms)
                {
                    int bf_curr = atom.B_factor;
                    int bf_delta = bf_curr - bf_prev;
                    bf_prev = bf_curr;

                    rc_encode_int((uint32_t)rc_int_class_t::bf_delta, dict_main_prefix, dict_main_suffix, bf_delta);
                }
        }
    }

    rc_encode_int((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix, 0);

    cart_precision = dynamic_cast<const NumericColumn*>(le->findColumn(StructFile::Columns::Cartn_x))->numDecimals;
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

                rc_encode_int((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix, col->numDecimals);
                rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix, col->width);
            }
            else 
            {
                const StringColumn* col = dynamic_cast<const StringColumn*>(c);

                rc_encode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix, col->width);
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
void StructCompressor::decompress(StructFile* output, const vector<uint8_t>& packed, size_t raw_size, const string& struct_name)
{
    clear_rc_dict();

    model_entry_type_t tpl_entry_type(rcd, nullptr, false);

    prepare_rc_tpl(false);

    ser_full.set_data(packed);

    ser_full.load_serializer(ser_plain);
    ser_full.load_serializer(ser_rc);

    v_rc.clear();
    v_rc.resize(ser_rc.load32u());
    vios->restart_read();
    ser_rc.load_bytes((char*)v_rc.data(), v_rc.size());

    rcd->start();

    // Decode AA sequences
    model_aa_t ctx_aa(rcd, nullptr, true);

    aa_sequences.resize(rc_decode_int((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix));
    for (auto& x : aa_sequences)
        x = (aa_t)ctx_aa.decode();

    aa_sequences_iter = aa_sequences.begin();

    extend_zstd_dict(struct_name);

    ser_plain.decompress_zstd(zstd_dctx, &zstd_dict_ext);

    Serializer ser_buffer;
    ser_plain.load_serializer(ser_strings);
    ser_plain.load_serializer(ser_text);

    output->reset(ser_buffer.size(), raw_size);

    char* str_data = output->getDataBuffer();
    ser_buffer.load_bytes(str_data, ser_buffer.size());

    ctx_entry_type = 0;

    while (true)
    {
        auto p_entry_type = rc_find_context(dict_entry_type, ctx_entry_type, &tpl_entry_type);
        entry_type_t entry_type = (entry_type_t)p_entry_type->decode();

        ctx_entry_type = ((ctx_entry_type << 3) + (uint8_t)entry_type) & 0x7u;

        if (entry_type == entry_type_t::block)
        {
            output->addEntry(decompress_block_entry(str_data));
        }
        else if (entry_type == entry_type_t::loop_generic)
        {
            output->addEntry(decompress_loop_generic(str_data));
        }
        else if (entry_type == entry_type_t::loop_atom)
        {
            output->addEntry(decompress_loop_atom(str_data));
        }
        else if (entry_type == entry_type_t::loop_hetatm)
        {
            //
        }
        else if (entry_type == entry_type_t::eod)
        {
            break;
        }
        else
            assert(0);          // Cannot be here
    }

    rcd->complete();

    clear_serializers();
}

// *****************************************************************
BlockEntry* StructCompressor::decompress_block_entry(char*& str_data)
{
    uint64_t entry_size = rc_decode_int((uint32_t)rc_int_class_t::block_entry_size, dict_main_prefix, dict_main_suffix);

    // make a new entry
    ser_text.load_bytes(str_data, entry_size+1);
    BlockEntry* newEntry = new BlockEntry("", entry_size, str_data);

    return newEntry;
}

// *****************************************************************
LoopEntry* StructCompressor::decompress_loop_generic(char*& str_data)
{
    string entry_name = ser_strings.load_str();

    uint64_t no_cols = rc_decode_int((uint32_t)rc_int_class_t::no_columns, dict_main_prefix, dict_main_suffix);
    uint64_t no_rows = rc_decode_int((uint32_t)rc_int_class_t::no_rows, dict_main_prefix, dict_main_suffix);

    LoopEntry* newEntry = new LoopEntry(entry_name, LoopEntry::Type::Standard);

    auto p_col_type = rc_find_context(dict_col_type, 0, tpl_col_type_dec);

    vector<int> v_tmp;

    for (uint64_t i = 0; i < no_cols; ++i)
    {
        col_type_t col_type = (col_type_t)p_col_type->decode();
        string col_name = ser_strings.load_str();

        if (col_type == col_type_t::numeric_general || col_type == col_type_t::numeric_same_values || col_type == col_type_t::numeric_same_diff)
            newEntry->addColumn(decompress_numeric_column(col_type, col_name, no_rows, v_tmp));
        else if(col_type == col_type_t::cart_as_numeric)
            newEntry->addColumn(decompress_cart_column_as_numeric(col_type, col_name, no_rows, v_tmp));
        else if (col_type == col_type_t::text)
            newEntry->addColumn(decompress_string_column(col_name, no_rows, str_data));
        else
            assert(0);           // Cannot be here
    }

    return newEntry;
}

// *****************************************************************
AbstractColumn* StructCompressor::decompress_string_column_special(const string& col_name, int no_rows, char*& str_data)
{
    uint8_t width = rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix);

    return new StringColumn(col_name, (int)no_rows, width, str_data);
}

// *****************************************************************
AbstractColumn* StructCompressor::decompress_string_column(const string& col_name, int no_rows, char*& str_data)
{
    uint8_t width = rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix);

    int tot_rep = 0;

    char* ptr = str_data;

    while (tot_rep < no_rows)
    {
        int no_reps = rc_decode_int((uint32_t)rc_int_class_t::string_rep, dict_main_prefix, dict_main_suffix) + 1;
        tot_rep += no_reps;

        char *p = ser_text.load_cstr();

        for (int i = 0; i < no_reps; ++i)
        {
            for (char* q = p; *q; ++q)
                *ptr++ = *q;
            *ptr++ = 0;
        }
    }

    return new StringColumn(col_name, (int)no_rows, width, str_data);
}

// *****************************************************************
AbstractColumn* StructCompressor::decompress_numeric_column(col_type_t col_type, const string& col_name, int no_rows, vector<int>& v_tmp)
{
    uint8_t decimals = rc_decode_int((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix);
    uint8_t width = rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix);

    v_tmp.clear();

    if (col_type == col_type_t::numeric_same_values)
    {
        int x = rc_decode_int((uint32_t)rc_int_class_t::numeric_val, dict_main_prefix, dict_main_suffix);
        v_tmp.resize(no_rows, x);
    }
    else if (col_type == col_type_t::numeric_same_diff)
    {
        int x = rc_decode_int((uint32_t)rc_int_class_t::numeric_val, dict_main_prefix, dict_main_suffix);
        int diff = rc_decode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_suffix);

        v_tmp.resize(no_rows);

        for (int i = 0; i < no_rows; ++i)
        {
            v_tmp[i] = x;
            x += diff;
        }
    }
    else
    {
        int p_val = 0;

        v_tmp.resize(no_rows);

        for (int i = 0; i < no_rows; ++i)
        {
            int x = rc_decode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_suffix);

            v_tmp[i] = x + p_val;
            p_val = x + p_val;
        }
    }

    return new NumericColumn(col_name, decimals, width, std::move(v_tmp));
}

// *****************************************************************
AbstractColumn* StructCompressor::decompress_cart_column_as_numeric(col_type_t col_type, const string& col_name, int no_rows, vector<int>& v_tmp)
{
    uint8_t decimals = rc_decode_int((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix);
    uint8_t width = rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix);

    cart_precision = rcd->get_cumulative_freq(7);
    rcd->update_frequency(1, cart_precision, 7);

    res_backbone_uA_single_axis = dec_resolution(rcd);

    int64_t resolution = res_backbone_uA_single_axis;

    cart_working_multiplier = pow10<int>(cart_working_precision - cart_precision);

    v_tmp.resize(no_rows);

    int p_val = 0;

    for (int i = 0; i < no_rows; ++i)
    {
        int x = rc_decode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_suffix);

        v_tmp[i] = (x + p_val) * resolution / cart_working_multiplier;
        p_val = x + p_val;
    }

    return new NumericColumn(col_name, decimals, width, std::move(v_tmp));
}

// *****************************************************************
void StructCompressor::append_cstr(char*& ptr, const char* src)
{
    while (*src)
        *ptr++ = *src++;

    *ptr++ = 0;
}

// *****************************************************************
LoopEntry* StructCompressor::decompress_loop_atom(char*& str_data)
{
    string loop_name = ser_strings.load_str();

    int no_cols = rc_decode_int((uint32_t)rc_int_class_t::no_columns, dict_main_prefix, dict_main_suffix);
    int no_rows = rc_decode_int((uint32_t)rc_int_class_t::no_rows, dict_main_prefix, dict_main_suffix);

    vector<chain_desc_t> chains;

    while (true)
    {
        int chain_len = rc_decode_int((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix);
        if (chain_len == 0)
            break;

        char chain_id = rcd->get_cumulative_freq(128);
        rcd->update_frequency(1, (int)chain_id, 128);

        bool is_constant_bf = (bool) rcd->get_cumulative_freq(2);
        rcd->update_frequency(1, (int)is_constant_bf, 2);

        bool is_oxt = (bool)rcd->get_cumulative_freq(2);
        rcd->update_frequency(1, (int)is_oxt, 2);

        chains.emplace_back(chain_desc_t{chain_id});
        auto& chain = chains.back();

        int id = 1;
        int seq_id = 1;
        int bf_prev = 0;
        int bf_curr = 0;

        chain.aa.resize(chain_len);
        for (int i = 0; i < chain_len; ++i)
        {
            auto& aa = chain.aa[i];
            aa.type = *aa_sequences_iter++;
            aa.type_str = aa_to_str(aa.type);
            aa.seq_id = seq_id++;

            if (is_constant_bf)
            {
                int bf_delta = rc_decode_int((uint32_t)rc_int_class_t::bf_delta, dict_main_prefix, dict_main_suffix);
                bf_curr = bf_prev + bf_delta;
                bf_prev = bf_curr;
            }

            const auto& aa_atoms = m_aa_atom.at(aa.type);

            aa.atoms.resize(aa_atoms.size() - (i + 1 != chain_len || !is_oxt));
            for (size_t j = 0; j < aa.atoms.size(); ++j)
            {
                auto& atom = aa.atoms[j];

                atom.type = aa_atoms[j];
                atom.id = id++;
                atom.type_str = atom_to_str(atom.type);
                atom.type_symbol = atom_type_char.at(atom.type);

                if (!is_constant_bf)
                {
                    int bf_delta = rc_decode_int((uint32_t)rc_int_class_t::bf_delta, dict_main_prefix, dict_main_suffix);
                    bf_curr = bf_prev + bf_delta;
                    bf_prev = bf_curr;
                }

                atom.B_factor = bf_curr;
            }
        }
    }

    int cart_precision = rcd->get_cumulative_freq(7);
    rcd->update_frequency(1, cart_precision, 7);

    cart_working_multiplier = pow10<int>(cart_working_precision - cart_precision);
    decompress_cart(chains);

    LoopEntry* newEntry = new LoopEntry(loop_name, LoopEntry::Type::Atom);

    vector<int> v_tmp;

    for (int i = 0; i < no_cols; ++i)
    {
        int col_id = rcd->get_cumulative_freq(chain_desc_col.size() + 1);
        rcd->update_frequency(1, (int)col_id, chain_desc_col.size() + 1);

        if (col_id == (int) chain_desc_col.size())
        {
            auto p_col_type = rc_find_context(dict_col_type, 0, tpl_col_type_dec);
            col_type_t col_type = (col_type_t)p_col_type->decode();

            string col_name = ser_strings.load_str();

            if (col_type == col_type_t::numeric_general || col_type == col_type_t::numeric_same_values || col_type == col_type_t::numeric_same_diff)
                newEntry->addColumn(decompress_numeric_column(col_type, col_name, no_rows, v_tmp));
            else if (col_type == col_type_t::text)
                newEntry->addColumn(decompress_string_column(col_name, no_rows, str_data));
            else
                assert(0);
        }
        else
        {
            set<string>::const_iterator iter = chain_desc_col.cbegin();
            advance(iter, col_id);

            const string col_name = *iter;

            if (chain_desc_col_numeric.count(col_name))
            {
                v_tmp.clear();
                v_tmp.reserve(no_rows);

                if (col_name == StructFile::Columns::id)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.id);
                else if (col_name == StructFile::Columns::auth_seq_id)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (size_t i = 0; i < aa.atoms.size(); ++i)
                                v_tmp.emplace_back(aa.seq_id);
                else if (col_name == StructFile::Columns::Cartn_x)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.coords.x);
                else if (col_name == StructFile::Columns::Cartn_y)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.coords.y);
                else if (col_name == StructFile::Columns::Cartn_z)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.coords.z);
                else if (col_name == StructFile::Columns::B_iso_or_equiv)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.B_factor);
                else
                    assert(0);

                uint8_t decimals = rc_decode_int((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix);
                uint8_t width = rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_suffix);
                newEntry->addColumn(new NumericColumn(col_name, decimals, width, std::move(v_tmp)));
            }
            else
            {
                char* ptr = str_data;

                if (col_name == StructFile::Columns::group_PDB)
                    for (int i = 0; i < no_rows; ++i)
                        append_cstr(ptr, "ATOM");
                else if (col_name == StructFile::Columns::auth_atom_id)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                append_cstr(ptr, atom.type_str.c_str());
                else if (col_name == StructFile::Columns::auth_comp_id)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (size_t i = 0; i < aa.atoms.size(); ++i)
                                append_cstr(ptr, aa.type_str.c_str());
                else if (col_name == StructFile::Columns::auth_asym_id)
                    for(const auto &chain: chains)
                        for(const auto &aa: chain.aa)
                            for (size_t i = 0; i < aa.atoms.size(); ++i)
                            {
                                *ptr++ = chain.id;
                                *ptr++ = 0;
                            }
                else if (col_name == StructFile::Columns::type_symbol)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                            {
                                *ptr++ = atom.type_symbol;
                                *ptr++ = 0;
                            }
                else
                    assert(0);

                newEntry->addColumn(decompress_string_column_special(col_name, no_rows, str_data));
            }
        }
    }

    return newEntry;
}

// *****************************************************************
LoopEntry* StructCompressor::decompress_loop_hetatm(char*& str_data)
{
    return nullptr;
}

// *****************************************************************
void StructCompressor::clear_serializers()
{
    ser_plain.clear();
    ser_strings.clear();
    ser_text.clear();
    ser_rc.clear();
    ser_full.clear();
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
    int64_t dx;
    int64_t dy;
    int64_t dz;

    rc_simple_fixed<vector_io_stream, 2, 1 << 15, 1> rce_ok(rce, nullptr, true);
//    rc_simple_fixed<vector_io_stream, 2, 1 << 15, 1> rce_first(rce, nullptr, true);

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

    cart_comp.clean_model();

//    map<int, string> str_first;

//    int cnt = 0;
    for (auto& chain : chains)
    {
        cart_comp.restart_chain();

        for (auto& aa : chain.aa)
        {
            cart_comp.new_aa(aa);

            for (size_t i = 0; i < aa.atoms.size(); ++i)
            {
                bool ok = cart_comp.predict((int) i, centroid_id, no_centroids, first, dx, dy, dz);

                rce_ok.encode((int)ok);

                uint32_t ctx = cart_comp.calc_ctx(aa.type, aa.atoms[i].type);

                if (ok)
                {
                    if (no_centroids > 1)
                    {
                        auto p_centroid_id = rc_find_context(dict_centroid_id, no_centroids, &tpl_centroid_id_enc[no_centroids]);
//                        auto p_centroid_id = rc_find_context(dict_centroid_id, 43 * (int)aa.type + (int) aa.atoms[i].type, &tpl_centroid_id_enc[no_centroids]);   // ecoli -12kB

                        p_centroid_id->encode(centroid_id);
                    }

                    auto p_first = rc_find_context(dict_tetrahedron_first, cart_comp.packed_atom_ctx(aa.type, aa.atoms[i].type), tpl_tetrahedron_first_enc);

/*                    if (first)
                        str_first[cart_comp.packed_atom_ctx(aa.type, aa.atoms[i].type)] += "+" + to_string(centroid_id) + " ";
                    else
                        str_first[cart_comp.packed_atom_ctx(aa.type, aa.atoms[i].type)] += "-" + to_string(centroid_id) + " ";*/

                    p_first->encode((int)first);
//                    rce_first.encode((int) first);
//                    ++cnt;

                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_suffix, dx);
                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_suffix, dy);
                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_suffix, dz);
                }
                else
                {
                    int64_t resolution = is_backbone_atom(aa.atoms[i].type) ? res_backbone_uA_single_axis : res_sidechain_uA_single_axis;

                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_suffix, aa.atoms[i].coords.x / resolution);
                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_suffix, aa.atoms[i].coords.y / resolution);
                    rc_encode_int(ctx, dict_cart_prefix, dict_cart_suffix, aa.atoms[i].coords.z / resolution);
                }
            }
        }
    }


/*    for (auto& x : str_first)
    {
        cout << x.first << " " << x.second << endl;
    }

    cout << endl;*/
}

// *****************************************************************
void StructCompressor::decompress_cart(vector<chain_desc_t>& chains)
{
    rc_simple_fixed<vector_io_stream, 2, 1 << 15, 1> rcd_ok(rcd, nullptr, false);
//    rc_simple_fixed<vector_io_stream, 2, 1 << 15, 1> rcd_first(rcd, nullptr, false);

    lossless = dec_flag(rcd);

    if (!lossless)
    {
        res_backbone_uA_single_axis = dec_resolution(rcd);
        res_sidechain_uA_single_axis = dec_resolution(rcd);
    }
    else
    {
        res_backbone_uA_single_axis = cart_working_multiplier;
        res_sidechain_uA_single_axis = cart_working_multiplier;
    }

    cart_decomp.set_resolution(res_backbone_uA_single_axis, res_sidechain_uA_single_axis);
    cart_decomp.clean_model();

    int centroid_id;
    int no_centroids;
    bool first;
    int64_t dx;
    int64_t dy;
    int64_t dz;

    for (auto& chain : chains)
    {
        cart_decomp.restart_chain();

        for (auto& aa : chain.aa)
        {
            cart_decomp.start_aa(aa.type);

            for (size_t i = 0; i < aa.atoms.size(); ++i)
            {
                int_coords_t decoded_coords;

                bool ok = (bool)rcd_ok.decode();

                uint32_t ctx = cart_decomp.calc_ctx(aa.type, aa.atoms[i].type);

                if (ok)
                {
                    no_centroids = (int) cart_decomp.get_no_centroids(aa.type, aa.atoms[i].type);

                    if (no_centroids > 1)
                    {
                        auto p_centroid_id = rc_find_context(dict_centroid_id, no_centroids, &tpl_centroid_id_dec[no_centroids]);
                        centroid_id = p_centroid_id->decode();
                    }
                    else
                        centroid_id = 0;

                    auto p_first = rc_find_context(dict_tetrahedron_first, cart_comp.packed_atom_ctx(aa.type, aa.atoms[i].type), tpl_tetrahedron_first_dec);

//                    first = (bool)rcd_first.decode();
                    first = (bool)p_first->decode();

                    dx = rc_decode_int(ctx, dict_cart_prefix, dict_cart_suffix);
                    dy = rc_decode_int(ctx, dict_cart_prefix, dict_cart_suffix);
                    dz = rc_decode_int(ctx, dict_cart_prefix, dict_cart_suffix);

                    cart_decomp.decode(aa.atoms[i].type, centroid_id, first, dx, dy, dz, decoded_coords);
                }
                else
                {
                    int64_t resolution = is_backbone_atom(aa.atoms[i].type) ? res_backbone_uA_single_axis : res_sidechain_uA_single_axis;

                    decoded_coords.x = rc_decode_int(ctx, dict_cart_prefix, dict_cart_suffix);
                    decoded_coords.y = rc_decode_int(ctx, dict_cart_prefix, dict_cart_suffix);
                    decoded_coords.z = rc_decode_int(ctx, dict_cart_prefix, dict_cart_suffix);

                    decoded_coords.x *= resolution;
                    decoded_coords.y *= resolution;
                    decoded_coords.z *= resolution;

                    cart_decomp.set_coords(aa.atoms[i].type, decoded_coords);
                }

                aa.atoms[i].coords.x = decoded_coords.x / cart_working_multiplier;
                aa.atoms[i].coords.y = decoded_coords.y / cart_working_multiplier;
                aa.atoms[i].coords.z = decoded_coords.z / cart_working_multiplier;
            }
        }
    }
}

// *****************************************************************
void StructCompressor::init_rc_types(bool compress)
{
    vios = new vector_io_stream(v_rc);

    if (compress)
        rce = new rc_encoder<vector_io_stream>(*vios);
    else
        rcd = new rc_decoder<vector_io_stream>(*vios);
}

// *****************************************************************
void StructCompressor::release_rc_types()
{
    delete_var(vios);
    delete_var(rce);
    delete_var(rcd);

    delete_var(tpl_prefix_enc);
    delete_var(tpl_col_type_enc);
    delete_var(tpl_tetrahedron_first_enc);
    tpl_suffix_enc.clear();
    tpl_centroid_id_enc.clear();

    delete_var(tpl_prefix_dec);
    delete_var(tpl_col_type_dec);
    delete_var(tpl_tetrahedron_first_dec);
    tpl_suffix_dec.clear();
    tpl_centroid_id_dec.clear();
}

// *****************************************************************
void StructCompressor::extend_zstd_dict(const string& struct_name)
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

// *****************************************************************
void StructCompressor::prepare_rc_tpl(bool compression)
{
    if (compression)
    {
        if (!tpl_col_type_enc)
            tpl_col_type_enc = new model_col_type_t(rce, nullptr, true);
        if (!tpl_prefix_enc)
            tpl_prefix_enc = new model_prefix_t(rce, nullptr, true);
        if (!tpl_tetrahedron_first_enc)
            tpl_tetrahedron_first_enc = new model_tetrahedron_first_t(rce, nullptr, true);

        if (tpl_suffix_enc.empty())
        {
            for (uint32_t i = 0; i <= prefix_short_max_bits; ++i)
                tpl_suffix_enc.push_back(model_suffix_t(rce, 1 << i, nullptr, true));
            tpl_suffix_enc.push_back(model_suffix_t(rce, 256, nullptr, true));
        }

        if(tpl_centroid_id_enc.empty())
            for (int i = 0; i <= cart_comp.max_no_centroids; ++i)
                tpl_centroid_id_enc.push_back(model_centroid_id_t(rce, i, nullptr, true));
    }
    else
    {
        if (!tpl_col_type_dec)
            tpl_col_type_dec = new model_col_type_t(rcd, nullptr, false);
        if (!tpl_prefix_dec)
            tpl_prefix_dec = new model_prefix_t(rcd, nullptr, false);
        if (!tpl_tetrahedron_first_dec)
            tpl_tetrahedron_first_dec = new model_tetrahedron_first_t(rcd, nullptr, false);

        if (tpl_suffix_dec.empty())
        {
            for (uint32_t i = 0; i <= prefix_short_max_bits; ++i)
                tpl_suffix_dec.push_back(model_suffix_t(rcd, 1 << i, nullptr, false));
            tpl_suffix_dec.push_back(model_suffix_t(rcd, 256, nullptr, false));
        }

        if (tpl_centroid_id_dec.empty())
            for (int i = 0; i <= cart_comp.max_no_centroids; ++i)
                tpl_centroid_id_dec.push_back(model_centroid_id_t(rcd, i, nullptr, false));
    }
}

// EOF
