# include "../compressors/struct-decompressor.h"

// *****************************************************************
int64_t StructDecompressor::rc_decode_uint(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec<model_suffix_t>& dict_suffix)
{
    bool is_neg = false;
    int val = 0;

    auto p_prefix = rc_find_context(dict_prefix, ctx, *tpl_prefix_dec);
    auto nb = p_prefix->decode();

    if (nb <= prefix_max_bits)
    {
        if (nb <= 1)
            return nb;

        if (nb < prefix_short_max_bits)
        {
            auto p_suffix = rc_find_context(dict_suffix, nb, &tpl_suffix_dec[nb - 1]);
            val = p_suffix->decode();
        }
        else
        {
            val = (int)rcd->get_cumulative_freq(1ull << (nb - 1));
            rcd->update_frequency(1, val, 1ull << (nb - 1));
        }

        val += (1 << (nb - 1));
    }
    else
    {
        uint32_t b1 = rc_find_context(dict_suffix, ctx * 16 + 13, &tpl_suffix_dec.back())->decode();
        uint32_t b2 = rc_find_context(dict_suffix, ctx * 16 + 14, &tpl_suffix_dec.back())->decode();
        uint32_t b3 = rc_find_context(dict_suffix, ctx * 16 + 15, &tpl_suffix_dec.back())->decode();
        uint32_t b4 = 0;

        if (b3 == 255)
        {
            b3 = rc_find_context(dict_suffix, ctx * 16 + 15, &tpl_suffix_dec.back())->decode();
            b4 = rc_find_context(dict_suffix, ctx * 16 + 15, &tpl_suffix_dec.back())->decode();
        }

        val = (b4 << 24) + (b3 << 16) + (b2 << 8) + b1;

        val += 1 << (prefix_max_bits - 1);
    }

    return val;
}

// *****************************************************************
int64_t StructDecompressor::rc_decode_int(uint32_t ctx, rc_context_vec_emb<model_prefix_t>& dict_prefix, rc_context_vec_emb<model_negative_t>& dict_negative, rc_context_vec<model_suffix_t>& dict_suffix)
{
    int64_t val = rc_decode_uint(ctx, dict_prefix, dict_suffix);

    if (val != 0)
    {
        /*        auto p_negative = rc_find_context(dict_negative, ctx, *tpl_negative_dec);
                if((bool)p_negative->decode())
                    val = -val;*/

        bool is_neg = (bool)rcd->get_cumulative_freq(2);
        rcd->update_frequency(1, (int)is_neg, 2);

        if (is_neg)
            val = -val;
    }

    return val;
}

// *****************************************************************
int64_t StructDecompressor::dec_resolution(rc_decoder<vector_io_stream>* rcd)
{
    /*    uint64_t x1, x2, x3;

        x1 = rcd->get_cumulative_freq(256);
        rcd->update_frequency(1, x1, 256);
        x2 = rcd->get_cumulative_freq(256);
        rcd->update_frequency(1, x2, 256);
        x3 = rcd->get_cumulative_freq(256);
        rcd->update_frequency(1, x3, 256);

        return (x3 << 16) + (x2 << 8) + x1;*/

    uint64_t r = rcd->get_cumulative_freq(1001);
    rcd->update_frequency(1, r, 1001);

    return r * 1000;
}

// *****************************************************************
bool StructDecompressor::dec_flag(rc_decoder<vector_io_stream>* rcd)
{
    bool flag = rcd->get_cumulative_freq(2);
    rcd->update_frequency(1, (int)flag, 2);

    return flag;
}

// *****************************************************************
void StructDecompressor::decompress(StructFileOutput* output, const vector<uint8_t>& packed, size_t raw_size, const string& struct_name)
{
    clear_rc_dict();

    model_entry_type_t tpl_entry_type(rcd, nullptr, false);

    prepare_rc_tpl();

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

    aa_sequences.resize(rc_decode_uint((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix));
    for (auto& x : aa_sequences)
        x = (aa_t)ctx_aa.decode();

    aa_sequences_iter = aa_sequences.begin();

    extend_zstd_dict(struct_name);
    ser_plain.decompress_zstd(zstd_dctx, &zstd_dict_ext);

//    Serializer ser_buffer;
    ser_plain.load_serializer(ser_strings);
    ser_plain.load_serializer(ser_text);

//    output->reset(ser_buffer.size(), raw_size);
    output->reset(raw_size, raw_size);

    char* str_data = output->getDataBuffer();
//    ser_buffer.load_bytes(str_data, ser_buffer.size());

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
BlockEntry* StructDecompressor::decompress_block_entry(char*& str_data)
{
    uint64_t entry_size = rc_decode_uint((uint32_t)rc_int_class_t::block_entry_size, dict_main_prefix, dict_main_suffix);

    // make a new entry
    ser_text.load_bytes(str_data, entry_size + 1);

    BlockEntry* newEntry = new BlockEntry("", entry_size, str_data);

    return newEntry;
}

// *****************************************************************
LoopEntry* StructDecompressor::decompress_loop_generic(char*& str_data)
{
    string entry_name = ser_strings.load_str();

    uint64_t no_cols = rc_decode_uint((uint32_t)rc_int_class_t::no_columns, dict_main_prefix, dict_main_suffix);
    uint64_t no_rows = rc_decode_uint((uint32_t)rc_int_class_t::no_rows, dict_main_prefix, dict_main_suffix);

    LoopEntry* newEntry = new LoopEntry(entry_name, LoopEntry::Type::Standard);

    auto p_col_type = rc_find_context(dict_col_type, 0, tpl_col_type_dec);

    vector<int> v_tmp;

    for (uint64_t i = 0; i < no_cols; ++i)
    {
        col_type_t col_type = (col_type_t)p_col_type->decode();
        string col_name = ser_strings.load_str();

        if (col_type == col_type_t::numeric_general || col_type == col_type_t::numeric_same_values || col_type == col_type_t::numeric_same_diff)
            newEntry->addColumn(decompress_numeric_column(col_type, col_name, (int) no_rows, v_tmp));
        else if (col_type == col_type_t::cart_as_numeric)
            newEntry->addColumn(decompress_cart_column_as_numeric(col_type, col_name, (int) no_rows, v_tmp));
        else if (col_type == col_type_t::text)
            newEntry->addColumn(decompress_string_column(col_name, (int) no_rows, str_data));
        else
            assert(0);           // Cannot be here
    }

    return newEntry;
}

// *****************************************************************
AbstractColumn* StructDecompressor::decompress_string_column_special(const string& col_name, int no_rows, char*& str_data)
{
    auto width = rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix);

    return new StringColumn(col_name, (int)no_rows, (int) width, str_data);
}

// *****************************************************************
AbstractColumn* StructDecompressor::decompress_string_column(const string& col_name, int no_rows, char*& str_data)
{
    auto width = rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix);

    int tot_rep = 0;

    char* ptr = str_data;

    while (tot_rep < no_rows)
    {
        auto no_reps = rc_decode_uint((uint32_t)rc_int_class_t::string_rep, dict_main_prefix, dict_main_suffix) + 1;
        tot_rep += (int) no_reps;

        char* p = ser_text.load_cstr();

        for (int i = 0; i < no_reps; ++i)
        {
            for (char* q = p; *q; ++q)
                *ptr++ = *q;
            *ptr++ = 0;
        }
    }

    return new StringColumn(col_name, (int)no_rows, (int) width, str_data);
}

// *****************************************************************
AbstractColumn* StructDecompressor::decompress_numeric_column(col_type_t col_type, const string& col_name, int no_rows, vector<int>& v_tmp)
{
    uint8_t decimals = (uint8_t) rc_decode_uint((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix);
    int8_t width = (int8_t) rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix);

    v_tmp.clear();

    if (col_type == col_type_t::numeric_same_values)
    {
        int x = (int) rc_decode_int((uint32_t)rc_int_class_t::numeric_val, dict_main_prefix, dict_main_negative, dict_main_suffix);
        v_tmp.resize(no_rows, x);
    }
    else if (col_type == col_type_t::numeric_same_diff)
    {
        int x = (int) rc_decode_int((uint32_t)rc_int_class_t::numeric_val, dict_main_prefix, dict_main_negative, dict_main_suffix);
        int diff = (int) rc_decode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_negative, dict_main_suffix);

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
            int x = (int) rc_decode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_negative, dict_main_suffix);

            v_tmp[i] = x + p_val;
            p_val = x + p_val;
        }
    }

    return new NumericColumn(col_name, decimals, width, std::move(v_tmp));
}

// *****************************************************************
AbstractColumn* StructDecompressor::decompress_cart_column_as_numeric(col_type_t col_type, const string& col_name, int no_rows, vector<int>& v_tmp)
{
    uint8_t decimals = (uint8_t) rc_decode_uint((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix);
    int8_t width = (int8_t) rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix);

    cart_precision = (int) rcd->get_cumulative_freq(7);
    rcd->update_frequency(1, cart_precision, 7);

    res_backbone_uA_single_axis = dec_resolution(rcd);

    int64_t resolution = res_backbone_uA_single_axis;

    cart_working_multiplier = pow10<int>(cart_working_precision - cart_precision);

    v_tmp.resize(no_rows);

    int p_val = 0;

    for (int i = 0; i < no_rows; ++i)
    {
        int x = (int) rc_decode_int((uint32_t)rc_int_class_t::numeric_diff, dict_main_prefix, dict_main_negative, dict_main_suffix);

        v_tmp[i] = (x + p_val) * resolution / cart_working_multiplier;
        p_val = x + p_val;
    }

    return new NumericColumn(col_name, decimals, width, std::move(v_tmp));
}

// *****************************************************************
void StructDecompressor::append_cstr(char*& ptr, const char* src)
{
    while (*src)
        *ptr++ = *src++;

    *ptr++ = 0;
}

// *****************************************************************
LoopEntry* StructDecompressor::decompress_loop_atom(char*& str_data)
{
    string loop_name = ser_strings.load_str();

    int no_cols = (int) rc_decode_uint((uint32_t)rc_int_class_t::no_columns, dict_main_prefix, dict_main_suffix);
    int no_rows = (int) rc_decode_uint((uint32_t)rc_int_class_t::no_rows, dict_main_prefix, dict_main_suffix);

    vector<chain_desc_t> chains;

    while (true)
    {
        int chain_len = (int) rc_decode_uint((uint32_t)rc_int_class_t::aa_sequence_len, dict_main_prefix, dict_main_suffix);
        if (chain_len == 0)
            break;

        char chain_id = rcd->get_cumulative_freq(128);
        rcd->update_frequency(1, (int)chain_id, 128);

        bool is_constant_bf = (bool)rcd->get_cumulative_freq(2);
        rcd->update_frequency(1, (int)is_constant_bf, 2);

        bool is_oxt = (bool)rcd->get_cumulative_freq(2);
        rcd->update_frequency(1, (int)is_oxt, 2);

        chains.emplace_back(chain_desc_t{ chain_id });
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
                int bf_delta = (int) rc_decode_int((uint32_t)rc_int_class_t::bf_delta, dict_main_prefix, dict_main_negative, dict_main_suffix);
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
                    int bf_delta = (int) rc_decode_int((uint32_t)rc_int_class_t::bf_delta, dict_main_prefix, dict_main_negative, dict_main_suffix);
                    bf_curr = bf_prev + bf_delta;
                    bf_prev = bf_curr;
                }

                atom.B_factor = bf_curr;
            }
        }
    }

    int cart_precision = (int) rcd->get_cumulative_freq(7);
    rcd->update_frequency(1, cart_precision, 7);

    cart_working_multiplier = pow10<int>(cart_working_precision - cart_precision);
    decompress_cart(chains);

    LoopEntry* newEntry = new LoopEntry(loop_name, LoopEntry::Type::Atom);

    vector<int> v_tmp;

    for (int i = 0; i < no_cols; ++i)
    {
        int col_id = (int) rcd->get_cumulative_freq(chain_desc_col.size() + 1);
        rcd->update_frequency(1, (int)col_id, chain_desc_col.size() + 1);

        if (col_id == (int)chain_desc_col.size())
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

                if (col_name == StructFileBase::Columns::id)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.id);
                else if (col_name == StructFileBase::Columns::auth_seq_id)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (size_t i = 0; i < aa.atoms.size(); ++i)
                                v_tmp.emplace_back(aa.seq_id);
                else if (col_name == StructFileBase::Columns::Cartn_x)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.coords.x);
                else if (col_name == StructFileBase::Columns::Cartn_y)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.coords.y);
                else if (col_name == StructFileBase::Columns::Cartn_z)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.coords.z);
                else if (col_name == StructFileBase::Columns::B_iso_or_equiv)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                v_tmp.emplace_back(atom.B_factor);
                else
                    assert(0);

                uint8_t decimals = (uint8_t) rc_decode_uint((uint32_t)rc_int_class_t::col_decimals, dict_main_prefix, dict_main_suffix);
                int8_t width = (int8_t) rc_decode_int((uint32_t)rc_int_class_t::col_width, dict_main_prefix, dict_main_negative, dict_main_suffix);
                newEntry->addColumn(new NumericColumn(col_name, decimals, width, std::move(v_tmp)));
            }
            else
            {
                char* ptr = str_data;

                if (col_name == StructFileBase::Columns::group_PDB)
                    for (int i = 0; i < no_rows; ++i)
                        append_cstr(ptr, "ATOM  ");
                else if (col_name == StructFileBase::Columns::auth_atom_id)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (const auto& atom : aa.atoms)
                                append_cstr(ptr, atom.type_str.c_str());
                else if (col_name == StructFileBase::Columns::auth_comp_id)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (size_t i = 0; i < aa.atoms.size(); ++i)
                                append_cstr(ptr, aa.type_str.c_str());
                else if (col_name == StructFileBase::Columns::auth_asym_id)
                    for (const auto& chain : chains)
                        for (const auto& aa : chain.aa)
                            for (size_t i = 0; i < aa.atoms.size(); ++i)
                            {
                                *ptr++ = chain.id;
                                *ptr++ = 0;
                            }
                else if (col_name == StructFileBase::Columns::type_symbol)
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
LoopEntry* StructDecompressor::decompress_loop_hetatm(char*& str_data)
{
    return nullptr;
}

// *****************************************************************
void StructDecompressor::decompress_cart(vector<chain_desc_t>& chains)
{
    model_ok_t rcd_ok(rcd, model_ok_init, false);

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
    if (cart_decomp.clean_model())
        remake_init_tpls();

    int centroid_id;
    int no_centroids;
    bool first;
    int64_t dx;
    int64_t dy;
    int64_t dz;

    for (auto& chain : chains)
    {
        uint32_t n_in_chain = 0;
        int_coords_t prev_atom_coords{ 0, 0, 0 };

        cart_decomp.restart_chain();

        for (auto& aa : chain.aa)
        {
            cart_decomp.start_aa(aa.type);

            for (size_t i = 0; i < aa.atoms.size(); ++i)
            {
                int_coords_t decoded_coords;

                bool ok = (n_in_chain++ > 2) ? (bool)rcd_ok.decode() : false;

                uint32_t ctx = cart_decomp.calc_ctx(aa.type, aa.atoms[i].type);

                if (ok)
                {
                    no_centroids = (int)cart_decomp.get_no_centroids(aa.type, aa.atoms[i].type);

                    if (no_centroids > 1)
                    {
                        //                        auto p_centroid_id = rc_find_context(dict_centroid_id, no_centroids, &tpl_centroid_id_dec[no_centroids]);

                        auto x = cart_decomp.packed_atom_ctx(aa.type, aa.atoms[i].type);
                        auto p_centroid_id = rc_find_context(dict_centroid_id, x, &tpl_centroid_id_init_dec[x]);

                        centroid_id = p_centroid_id->decode();
                    }
                    else
                        centroid_id = 0;

                    dx = rc_decode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix);
                    dy = rc_decode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix);
                    dz = rc_decode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix);

                    pair<int_coords_t, int_coords_t> q_both;

                    cart_decomp.decode_part1(aa.atoms[i].type, centroid_id, q_both);

                    bool equal_q = q_both.first.x == q_both.second.x && q_both.first.y == q_both.second.y && q_both.first.z == q_both.second.z;

                    if (!equal_q)
                    {
                        auto p_first = rc_find_context(dict_tetrahedron_first, cart_decomp.packed_atom_ctx(aa.type, aa.atoms[i].type), tpl_tetrahedron_first_dec);
                        first = (bool)p_first->decode();
                    }
                    else
                        first = true;

                    cart_decomp.decode_part2(aa.atoms[i].type, first ? q_both.first : q_both.second, dx, dy, dz, decoded_coords);
                }
                else
                {
                    int64_t resolution = is_backbone_atom(aa.atoms[i].type) ? res_backbone_uA_single_axis : res_sidechain_uA_single_axis;

                    decoded_coords.x = rc_decode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix);
                    decoded_coords.y = rc_decode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix);
                    decoded_coords.z = rc_decode_int(ctx, dict_cart_prefix, dict_cart_negative, dict_cart_suffix);

                    decoded_coords.x *= resolution;
                    decoded_coords.y *= resolution;
                    decoded_coords.z *= resolution;

                    decoded_coords.x += prev_atom_coords.x;
                    decoded_coords.y += prev_atom_coords.y;
                    decoded_coords.z += prev_atom_coords.z;

                    cart_decomp.set_coords(aa.atoms[i].type, decoded_coords);
                }

                prev_atom_coords = decoded_coords;

                aa.atoms[i].coords.x = decoded_coords.x / cart_working_multiplier;
                aa.atoms[i].coords.y = decoded_coords.y / cart_working_multiplier;
                aa.atoms[i].coords.z = decoded_coords.z / cart_working_multiplier;
            }
        }
    }
}

// *****************************************************************
void StructDecompressor::init_rc_types()
{
    vios = new vector_io_stream(v_rc);

    rcd = new rc_decoder<vector_io_stream>(*vios);
}

// *****************************************************************
void StructDecompressor::release_rc_types()
{
    delete_var(vios);
    delete_var(rcd);

    delete_var(tpl_prefix_dec);
    delete_var(tpl_negative_dec);
    delete_var(tpl_col_type_dec);
    delete_var(tpl_tetrahedron_first_dec);
    tpl_suffix_dec.clear();
    tpl_centroid_id_dec.clear();
    tpl_centroid_id_init_dec.clear();
}

// *****************************************************************
void StructDecompressor::prepare_rc_tpl()
{
    if (!tpl_col_type_dec)
        tpl_col_type_dec = new model_col_type_t(rcd, nullptr, false);
    if (!tpl_prefix_dec)
        tpl_prefix_dec = new model_prefix_t(rcd, nullptr, false);
    if (!tpl_negative_dec)
        tpl_negative_dec = new model_negative_t(rcd, nullptr, false);
    if (!tpl_tetrahedron_first_dec)
        tpl_tetrahedron_first_dec = new model_tetrahedron_first_t(rcd, nullptr, false);

    if (tpl_suffix_dec.empty())
    {
        for (uint32_t i = 0; i <= prefix_short_max_bits; ++i)
            tpl_suffix_dec.push_back(model_suffix_t(rcd, 1 << i, nullptr, false));
        tpl_suffix_dec.push_back(model_suffix_t(rcd, 256, nullptr, false));
    }

    if (tpl_centroid_id_dec.empty())
        for (int i = 0; i <= cart_decomp.max_no_centroids; ++i)
            tpl_centroid_id_dec.push_back(model_centroid_id_t(rcd, i, nullptr, false));

    if (tpl_centroid_id_init_dec.empty())
        for (int i = 0; i <= (int) cart_decomp.max_packed_atom_ctx; ++i)
        {
            auto cc = cart_decomp.get_centroid_counts(i);

            if (cc.first)
                tpl_centroid_id_init_dec.push_back(model_centroid_id_t(rcd, (uint32_t) cc.second, cc.first, false));
            else
                tpl_centroid_id_init_dec.push_back(model_centroid_id_t(rcd, 2, nullptr, false));     /// will not be used
        }
}

// *****************************************************************
void StructDecompressor::remake_init_tpls()
{
    tpl_centroid_id_init_dec.clear();

    for (int i = 0; i <= (int) cart_decomp.max_packed_atom_ctx; ++i)
    {
        auto cc = cart_decomp.get_centroid_counts(i);

        if (cc.first)
            tpl_centroid_id_init_dec.push_back(model_centroid_id_t(rcd, (uint32_t) cc.second, cc.first, false));
        else
            tpl_centroid_id_init_dec.push_back(model_centroid_id_t(rcd, 2, nullptr, false));     // will not be used
    }
}
